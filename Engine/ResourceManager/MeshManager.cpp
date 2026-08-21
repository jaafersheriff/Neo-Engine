#include "MeshManager.hpp"

#include "Loader/MeshGenerator.hpp"

#include "Util/Profiler.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	struct MeshLoader final {

		std::optional<CachedResource<Mesh>> load(MeshLoadDetails meshDetails, const std::optional<std::string>& debugName) const {
			if (debugName.has_value()) {
				NEO_LOG_V("Uploading mesh %s", debugName.value().c_str());
			}
			CachedResource<Mesh> meshResource(meshDetails.mPrimtive);
			meshResource.mResource.init(debugName);
			for (auto&& [type, buffer] : meshDetails.mVertexBuffers) {
				meshResource.mResource.addVertexBuffer(
					type,
					buffer.mComponents,
					buffer.mStride,
					buffer.mFormat,
					buffer.mNormalized,
					buffer.mCount,
					buffer.mOffset,
					buffer.mByteSize,
					buffer.mData
				);
			}
			if (meshDetails.mElementBuffer) {
				meshResource.mResource.addElementBuffer(
					meshDetails.mElementBuffer->mCount,
					meshDetails.mElementBuffer->mFormat,
					meshDetails.mElementBuffer->mByteSize,
					meshDetails.mElementBuffer->mData
				);
			}
			meshResource.mDebugName = debugName;
			return meshResource;
		}
	};

	MeshManager::MeshManager() {
	}

	void MeshManager::_initImpl() {
		auto cubeDetails = prefabs::generateCube();
		std::optional<CachedResource<Mesh>> fallback = MeshLoader{}.load(*cubeDetails, "Fallback Cube");
		NEO_ASSERT(fallback.has_value(), "Failed to load the fallback mesh");
		mFallback = std::make_shared<CachedResource<Mesh>>(std::move(*fallback));
		for (auto&& [type, buffer] : cubeDetails->mVertexBuffers) {
			delete[] buffer.mData;
		}
		if (cubeDetails->mElementBuffer) {
			delete[] cubeDetails->mElementBuffer->mData;
		}
	}

	MeshManager::~MeshManager() {
		mFallback.reset();
	}

	[[nodiscard]] MeshHandle MeshManager::_asyncLoadImpl(MeshHandle id, MeshLoadDetails meshDetails, const std::optional<std::string>& debugName) const {
		if (debugName.has_value()) {
			NEO_LOG_V("Loading mesh %s", debugName->c_str());
		}

		// Copy data so this can be ticked next frame
		MeshLoadDetails copy = meshDetails;
		for (auto&& [type, buffer] : meshDetails.mVertexBuffers) {
			if (buffer.mData) {
				copy.mVertexBuffers[type].mData = new uint8_t[buffer.mByteSize];
				memcpy(const_cast<uint8_t*>(copy.mVertexBuffers[type].mData), buffer.mData, buffer.mByteSize);
			}
		}
		if (meshDetails.mElementBuffer.has_value() && meshDetails.mElementBuffer->mData) {
			copy.mElementBuffer->mData = new uint8_t[meshDetails.mElementBuffer->mByteSize];
			memcpy(const_cast<uint8_t*>(copy.mElementBuffer->mData), meshDetails.mElementBuffer->mData, meshDetails.mElementBuffer->mByteSize);
		}

		{
			std::lock_guard<std::mutex> lock(mLoadQueueMutex);
			mLoadQueue.emplace_back(ResourceLoadDetails_Internal{ id,  copy, debugName });
		}

		return id;
	}

	void MeshManager::_tickImpl() {
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
				if (std::optional<CachedResource<Mesh>> mesh = MeshLoader{}.load(details.mLoadDetails, details.mDebugName)) {
					mCache.insert(details.mHandle, std::move(*mesh));
				}
				else {
					NEO_LOG_E("Failed to load mesh %s", details.mDebugName.value_or("").c_str());
				}
				// Published (or failed) - the claim taken in asyncLoad ends here, not when it left the queue.
				_finishPending(details.mHandle);
				for (auto&& [type, buffer] : details.mLoadDetails.mVertexBuffers) {
					delete[] buffer.mData;
				}
				if (details.mLoadDetails.mElementBuffer.has_value()) {
					delete[] details.mLoadDetails.mElementBuffer->mData;
				}
			}
		}

		{
			std::vector<std::pair<MeshHandle, std::function<void(Mesh&)>>> swapQueue;
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
						NEO_LOG_E("Attempting to transact on an invalid mesh");
					}
				}
			}
		}

		{
			std::vector<MeshHandle> swapQueue;
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
						retire(id);
					}
				}
			}
		}
	}

	void MeshManager::_destroyImpl(CachedResource<Mesh>& mesh) {
		mesh.mResource.destroy();
	}

	void MeshManager::imguiEditor() {
		mCache.forEach([](const CachedResource<Mesh>& mesh) {
			if (mesh.mDebugName.has_value()) {
				ImGui::Text("%s", mesh.mDebugName->c_str());
			}
			else {
				ImGui::Text("%d", mesh.mHandle.mHandle);
			}
		});
	}
}
