#include "MeshManager.hpp"

#include "Loader/MeshGenerator.hpp"

#include "Util/Profiler.hpp"
#include "Util/RenderThread.hpp"
#include "Util/ServiceLocator.hpp"

#include <imgui.h>

namespace neo {

	struct MeshLoader final : entt::resource_loader<MeshLoader, BackedResource<Mesh>> {

		std::shared_ptr<BackedResource<Mesh>> load(MeshLoadDetails meshDetails, const std::optional<std::string>& debugName) const {
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
	};

	MeshManager::MeshManager() {
		ServiceLocator<RenderThread>::ref().pushRenderFunc([this]() {
			auto cubeDetails = prefabs::generateCube();
			mFallback = MeshLoader{}.load(*cubeDetails, "Fallback Cube");
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
				copy.mVertexBuffers[type].mData = static_cast<uint8_t*>(malloc(buffer.mByteSize));
				memcpy(const_cast<uint8_t*>(copy.mVertexBuffers[type].mData), buffer.mData, buffer.mByteSize);
			}
		}
		if (meshDetails.mElementBuffer.has_value() && meshDetails.mElementBuffer->mData) {
			copy.mElementBuffer->mData = static_cast<uint8_t*>(malloc(meshDetails.mElementBuffer->mByteSize));
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
			for (auto& details : swapQueue) {
				renderThread.pushRenderFunc([this, details]() {
					{
						std::lock_guard<std::mutex> lock(mCacheMutex);
						mCache.load<MeshLoader>(details.mHandle.mHandle, details.mLoadDetails, details.mDebugName);
					}
					for (auto&& [type, buffer] : details.mLoadDetails.mVertexBuffers) {
						free(const_cast<uint8_t*>(buffer.mData));
					}
					if (details.mLoadDetails.mElementBuffer.has_value()) {
						free(const_cast<uint8_t*>(details.mLoadDetails.mElementBuffer->mData));
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

			for (auto&& [handle, func] : swapQueue) {
				renderThread.pushRenderFunc([this, handle, func]() {
					if (isValid(handle)) {
						std::lock_guard<std::mutex> lock(mCacheMutex);
						func(mCache.handle(handle.mHandle).get().mResource);
					}
					else {
						NEO_LOG_E("Attempting to transact on an invalid mesh");
					}
				});
			}
		}

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

			for (auto& id : swapQueue) {
				renderThread.pushRenderFunc([this, id]() {
					if (isValid(id)) {
						std::lock_guard<std::mutex> lock(mCacheMutex);
						mCache.discard(id.mHandle);
						_destroyImpl(mCache.handle(id.mHandle).get());
					}
				});
			}
		}
	}

	void MeshManager::_destroyImpl(BackedResource<Mesh>& mesh) {
		mesh.mResource.destroy();
	}

	void MeshManager::imguiEditor() {
		std::lock_guard<std::mutex> lock(mCacheMutex);
		mCache.each([](const MeshHandle id, const BackedResource<Mesh>& mesh) {
			NEO_UNUSED(mesh);
			if (mesh.mDebugName.has_value()) {
				ImGui::Text(mesh.mDebugName->c_str());
			}
			else {
				ImGui::Text("%d", id.mHandle);
			}
		});
	}
}