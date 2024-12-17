#include "MeshManager.hpp"

#include "Loader/MeshGenerator.hpp"

#include "Util/Profiler.hpp"

#include <ext/imgui_incl.hpp>

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
		auto cubeDetails = prefabs::generateCube();
		mFallback = MeshLoader{}.load(*cubeDetails, "Fallback Cube");
		for (auto&& [type, buffer] : cubeDetails->mVertexBuffers) {
			free(const_cast<uint8_t*>(buffer.mData));
		}
		if (cubeDetails->mElementBuffer) {
			free(const_cast<uint8_t*>(cubeDetails->mElementBuffer->mData));
		}
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
				mCache.load<MeshLoader>(details.mHandle.mHandle, details.mLoadDetails, details.mDebugName);
				for (auto&& [type, buffer] : details.mLoadDetails.mVertexBuffers) {
					free(const_cast<uint8_t*>(buffer.mData));
				}
				if (details.mLoadDetails.mElementBuffer.has_value()) {
					free(const_cast<uint8_t*>(details.mLoadDetails.mElementBuffer->mData));
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

			TRACY_GPUN("Transact");
			for (auto&& [handle, func] : swapQueue) {
				TRACY_GPUN("Transact Single");
				if (isValid(handle)) {
					func(mCache.handle(handle.mHandle).get().mResource);
				}
				else {
					NEO_LOG_E("Attempting to transact on an invalid mesh");
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

	void MeshManager::_destroyImpl(BackedResource<Mesh>& mesh) {
		mesh.mResource.destroy();
	}

	void MeshManager::imguiEditor() {
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
