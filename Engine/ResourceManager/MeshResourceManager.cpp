#include "MeshResourceManager.hpp"

#include "Loader/MeshGenerator.hpp"

#include "Util/Profiler.hpp"

#include <imgui.h>

namespace neo {

	struct MeshLoader final : entt::resource_loader<MeshLoader, BackedResource<Mesh>> {

		std::shared_ptr<BackedResource<Mesh>> load(MeshLoadDetails meshDetails, std::optional<std::string> debugName) const {
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

	MeshResourceManager::MeshResourceManager() {
		auto cubeDetails = prefabs::generateCube();
		mFallback = MeshLoader{}.load(*cubeDetails, "Fallback Cube");
		for (auto&& [type, buffer] : cubeDetails->mVertexBuffers) {
			free(const_cast<uint8_t*>(buffer.mData));
		}
		if (cubeDetails->mElementBuffer) {
			free(const_cast<uint8_t*>(cubeDetails->mElementBuffer->mData));
		}
	}

	MeshResourceManager::~MeshResourceManager() {
		mFallback->mResource.destroy();
		mFallback.reset();
	}

	[[nodiscard]] MeshHandle MeshResourceManager::_asyncLoadImpl(MeshHandle id, MeshLoadDetails meshDetails, std::optional<std::string> debugName) const {
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

		mQueue.emplace_back(ResourceLoadDetails_Internal{ id,  copy, debugName });

		return id;
	}

	void MeshResourceManager::_tickImpl() {
		TRACY_ZONE();

		{
			std::vector<ResourceLoadDetails_Internal> swapQueue = {};
			std::swap(mQueue, swapQueue);
			mQueue.clear();

			for (auto& details : swapQueue) {
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
			std::swap(mTransactionQueue, swapQueue);
			mTransactionQueue.clear();

			for (auto&& [handle, func] : swapQueue) {
				if (isValid(handle)) {
					func(resolve(handle));
				}
				else {
					NEO_LOG_E("Attempting to transact on an invalid mesh");
				}
			}
		}

		{
			std::vector<MeshHandle> swapQueue;
			std::swap(mDiscardQueue, swapQueue);
			mDiscardQueue.clear();

			for (auto& id : swapQueue) {
				if (isValid(id)) {
					mCache.handle(id.mHandle).get().mResource.destroy();
					mCache.discard(id.mHandle);
				}
			}
		}
	}

	void MeshResourceManager::_clearImpl() {
		mQueue.clear();
		mCache.each([](BackedResource<Mesh>& mesh) {
			mesh.mResource.destroy();
		});
		mCache.clear();
	}

	void MeshResourceManager::imguiEditor() {
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