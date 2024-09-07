#include "MeshManager.hpp"

#include "Loader/MeshGenerator.hpp"

#include "Util/Profiler.hpp"
#include "Util/RenderThread.hpp"
#include "Util/ServiceLocator.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	MeshManager::MeshManager() {
		ServiceLocator<RenderThread>::value().pushRenderFunc([this]() {
			auto cubeDetails = prefabs::generateCube();
			mFallback = MeshLoader()(*cubeDetails, "Fallback Cube");
			for (auto&& [type, buffer] : cubeDetails->mVertexBuffers) {
				free(const_cast<uint8_t*>(buffer.mData));
			}
			if (cubeDetails->mElementBuffer) {
				free(const_cast<uint8_t*>(cubeDetails->mElementBuffer->mData));
			}
		});
	}

	MeshManager::~MeshManager() {
		mFallback->mResource.destroy();
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
			std::lock_guard<std::mutex> lock(mQueueMutex);
			mQueue.emplace_back(ResourceLoadDetails_Internal{ id,  copy, debugName });
		}

		return id;
	}

	void MeshManager::_tickImpl(RenderThread& renderThread) {
		TRACY_ZONE();

		// Delete
		{
			std::vector<MeshHandle> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mDiscardMutex);
				if (!mDiscardQueue.empty()) {
					std::swap(mDiscardQueue, swapQueue);
					mDiscardQueue.clear();
				}
			}

			if (!swapQueue.empty()) {
				renderThread.pushRenderFunc([this, swapQueue = std::move(swapQueue)]() {
					TRACY_GPUN("MeshManager::Delete");
					for (auto& id : swapQueue) {
						if (isValid(id)) {
							std::lock_guard<std::mutex> lock(mCacheMutex);
							_destroyImpl(mCache[id.mHandle]);
							mCache.erase(id.mHandle);
						}
					}
				});
			}
		}


		// Create
		{
			std::vector<ResourceLoadDetails_Internal> swapQueue = {};
			{
				std::lock_guard<std::mutex> lock(mQueueMutex);
				if (!mQueue.empty()) {
					std::swap(mQueue, swapQueue);
					mQueue.clear();
				}
			}
			if (!swapQueue.empty()) {
				renderThread.pushRenderFunc([this, swapQueue = std::move(swapQueue)]() {
					TRACY_GPUN("MeshManager::Create");
					for (auto& details : swapQueue) {
						{
							std::lock_guard<std::mutex> lock(mCacheMutex);
							mCache.load(details.mHandle.mHandle, details.mLoadDetails, details.mDebugName);
						}
						for (auto&& [type, buffer] : details.mLoadDetails.mVertexBuffers) {
							free(const_cast<uint8_t*>(buffer.mData));
						}
						if (details.mLoadDetails.mElementBuffer.has_value()) {
							free(const_cast<uint8_t*>(details.mLoadDetails.mElementBuffer->mData));
						}
					}
				});
			}
		}

		// Update
		{
			std::vector<std::pair<MeshHandle, std::function<void(Mesh&)>>> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mTransactionMutex);
				if (!mTransactionQueue.empty()) {
					std::swap(mTransactionQueue, swapQueue);
					mTransactionQueue.clear();
				}
			}

			if (!swapQueue.empty()) {
				renderThread.pushRenderFunc([this, queue = std::move(swapQueue)]() {
					TRACY_GPUN("MeshManager::Update");
					for (auto&& [handle, func] : queue) {
						if (isValid(handle)) {
							std::lock_guard<std::mutex> lock(mCacheMutex);
							func(mCache[handle.mHandle]->mResource);
						}
						else {
							NEO_LOG_E("Attempting to transact on an invalid mesh");
						}
					}
				});
			}
		}
	}

	void MeshManager::_destroyImpl(BackedResource<Mesh>& mesh) {
		TRACY_GPU();
		NEO_ASSERT(ServiceLocator<RenderThread>::value().isRenderThread(), "Only call this from render thread");
		mesh.mResource.destroy();
	}

	void MeshManager::imguiEditor() {
		std::lock_guard<std::mutex> lock(mCacheMutex);
		for (auto [handle, mesh] : mCache) {
			if (mesh->mDebugName.has_value()) {
				ImGui::Text(mesh->mDebugName->c_str());
			}
			else {
				ImGui::Text("%d", handle);
			}
		}
	}

	std::shared_ptr<BackedResource<Mesh>> MeshLoader::operator()(MeshLoadDetails meshDetails, const std::optional<std::string>& debugName) const {
		TRACY_GPU();
		NEO_ASSERT(ServiceLocator<RenderThread>::value().isRenderThread(), "Only call this from render thread");
		if (debugName.has_value()) {
			NEO_LOG_V("Uploading mesh %s", debugName.value().c_str());
		}
		std::shared_ptr<BackedResource<Mesh>> meshResource = std::make_shared<BackedResource<Mesh>>(meshDetails.mPrimtive);
		meshResource->mResource.init(debugName);
		for (auto&& [type, buffer] : meshDetails.mVertexBuffers) {
			meshResource->mResource.addVertexBuffer(
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
			meshResource->mResource.addElementBuffer(
				meshDetails.mElementBuffer->mCount,
				meshDetails.mElementBuffer->mFormat,
				meshDetails.mElementBuffer->mByteSize,
				meshDetails.mElementBuffer->mData
			);
		}
		meshResource->mDebugName = debugName;
		return meshResource;
	}
}
