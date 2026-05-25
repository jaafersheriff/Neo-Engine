#include "ShaderBufferManager.hpp"

#include "Util/Profiler.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	struct ShaderBufferLoader final : entt::resource_loader<ShaderBufferLoader, BackedResource<ShaderBuffer>> {

		std::shared_ptr<BackedResource<ShaderBuffer>> load(ShaderBufferLoadDetails details, const std::optional<std::string>& debugName) const {
			if (debugName.has_value()) {
				NEO_LOG_V("Uploading shader buffer %s", debugName.value().c_str());
			}
			NEO_ASSERT(details.mByteSize != 0, "Empty shader buffer");
			if (details.mByteSize > 0) {
				std::shared_ptr<BackedResource<ShaderBuffer>> resource = std::make_shared<BackedResource<ShaderBuffer>>(details.mByteSize, details.mData, debugName);
				resource->mDebugName = debugName;
				return resource;
			}

			return nullptr;
		}
	};

	ShaderBufferManager::ShaderBufferManager() {
		// Small fallback buffer
		ShaderBufferLoadDetails fallbackDetails;
		fallbackDetails.mByteSize = 16;
		std::vector<uint8_t> data(16);
		std::fill(data.begin(), data.end(), 0u);
		fallbackDetails.mData = data.data();
		mFallback = ShaderBufferLoader{}.load(fallbackDetails, "Fallback ShaderBuffer");
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
		if (details.mData) {
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
				mCache.load<ShaderBufferLoader>(details.mHandle.mHandle, details.mLoadDetails, details.mDebugName);
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
						func(mCache.handle(handle.mHandle).get().mResource);
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
						_destroyImpl(mCache.handle(id.mHandle).get());
						mCache.discard(id.mHandle);
					}
				}
			}
		}
	}

	void ShaderBufferManager::_destroyImpl(BackedResource<ShaderBuffer>& buffer) {
		buffer.mResource.destroy();
	}

	void ShaderBufferManager::imguiEditor() {
		const uint32_t kb = 1024;
		const uint32_t mb = kb * 1024;
		mCache.each([kb, mb](const ShaderBufferHandle id, const BackedResource<ShaderBuffer>& buffer) {
			char buf[64];
			util::stringifyByteSize(buffer.mResource.mByteSize, buf, sizeof(buf));

			if (buffer.mDebugName.has_value()) {
				ImGui::Text("%s (%s)", buffer.mDebugName->c_str(), buf);
			}
			else {
				ImGui::Text("%d (%s)", id.mHandle, buf);
			}
			});
	}
}
