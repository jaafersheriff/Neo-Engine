#include "ShaderBufferManager.hpp"

#include "Util/Profiler.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	struct ShaderBufferLoader final {

		std::optional<CachedResource<ShaderBuffer>> load(ShaderBufferLoadDetails details, const std::optional<std::string>& debugName) const {
			if (debugName.has_value()) {
				NEO_LOG_V("Uploading shader buffer %s", debugName.value().c_str());
			}
			NEO_ASSERT(details.mByteSize != 0 && details.mData != nullptr, "Empty shader buffer");
			if (details.mByteSize == 0) {
				return std::nullopt;
			}

			CachedResource<ShaderBuffer> resource(details.mByteSize, details.mData, debugName);
			resource.mDebugName = debugName;
			return resource;
		}
	};

	ShaderBufferManager::ShaderBufferManager() {
		// Small fallback buffer
		ShaderBufferLoadDetails fallbackDetails;
		fallbackDetails.mByteSize = 16;
		std::vector<uint8_t> data(16);
		std::fill(data.begin(), data.end(), 0u);
		fallbackDetails.mData = data.data();
		std::optional<CachedResource<ShaderBuffer>> fallback = ShaderBufferLoader{}.load(fallbackDetails, "Fallback ShaderBuffer");
		NEO_ASSERT(fallback.has_value(), "Failed to load the fallback shader buffer");
		mFallback = std::make_shared<CachedResource<ShaderBuffer>>(std::move(*fallback));
	}

	ShaderBufferManager::~ShaderBufferManager() {
		mFallback->mResource.destroy();
		mFallback.reset();
	}

	[[nodiscard]] ShaderBufferHandle ShaderBufferManager::_asyncLoadImpl(ShaderBufferHandle id, ShaderBufferLoadDetails details, const std::optional<std::string>& debugName) const {
		if (debugName.has_value()) {
			NEO_LOG_V("Loading shader buffer %s", debugName->c_str());
		}

		// Copy data so this can be ticked next frame
		ShaderBufferLoadDetails copy = details;
		if (details.mData && details.mByteSize > 0) {
			copy.mData = new uint8_t[details.mByteSize];
			memcpy(const_cast<uint8_t*>(copy.mData), details.mData, details.mByteSize);
		}

		{
			std::lock_guard<std::mutex> lock(mLoadQueueMutex);
			mLoadQueue.emplace_back(ResourceLoadDetails_Internal{ id, copy, debugName });
		}

		return id;
	}

	void ShaderBufferManager::_tickImpl() {
		TRACY_ZONE();

		{
			std::vector<ResourceLoadDetails_Internal> swapQueue = {};
			{
				std::lock_guard<std::mutex> lock(mLoadQueueMutex);
				std::swap(mLoadQueue, swapQueue);
				mLoadQueue.clear();
			}
			TRACY_GPUN("Load");
			for (auto& details : swapQueue) {
				TRACY_GPUN("Create Single");
				if (std::optional<CachedResource<ShaderBuffer>> buffer = ShaderBufferLoader{}.load(details.mLoadDetails, details.mDebugName)) {
					mCache.insert(details.mHandle, std::move(*buffer));
				}
				else {
					NEO_LOG_E("Failed to load shader buffer %s", details.mDebugName.value_or("").c_str());
				}
				delete[] details.mLoadDetails.mData;
			}
		}

		{
			std::vector<std::pair<ShaderBufferHandle, std::function<void(ShaderBuffer&)>>> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mTransactionQueueMutex);
				std::swap(mTransactionQueue, swapQueue);
				mTransactionQueue.clear();
			}

			if (!swapQueue.empty()) {
				TRACY_GPUN("Transact");
				for (auto&& [handle, func] : swapQueue) {
					TRACY_GPUN("Transact Single");
					if (isValid(handle)) {
						func(mCache.resolve(handle)->mResource);
					}
					else {
						NEO_LOG_E("Attempting to transact on an invalid shader buffer");
					}
				}
			}
		}

		{
			std::vector<ShaderBufferHandle> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mDiscardQueueMutex);
				std::swap(mDiscardQueue, swapQueue);
				mDiscardQueue.clear();
			}

			if (!swapQueue.empty()) {
				TRACY_GPUN("Destroy");
				for (auto& id : swapQueue) {
					TRACY_GPUN("Destroy Single");
					if (isValid(id)) {
						_destroyImpl(*mCache.resolve(id));
						mCache.erase(id);
					}
				}
			}
		}
	}

	void ShaderBufferManager::_destroyImpl(CachedResource<ShaderBuffer>& buffer) {
		buffer.mResource.destroy();
	}

	void ShaderBufferManager::imguiEditor() {
		mCache.forEach([](const CachedResource<ShaderBuffer>& buffer) {
			if (buffer.mDebugName.has_value()) {
				ImGui::Text("%s (%u bytes)", buffer.mDebugName->c_str(), buffer.mResource.mByteSize);
			}
			else {
				ImGui::Text("%d (%u bytes)", buffer.mHandle.mHandle, buffer.mResource.mByteSize);
			}
		});
	}
}
