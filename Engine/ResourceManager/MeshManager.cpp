#include "MeshManager.hpp"

#include "Loader/MeshGenerator.hpp"
#include "ECS/ECS.hpp"
#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "Util/Profiler.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	MeshManager::MeshManager() {
		auto cubeDetails = prefabs::generateCube();
		mFallback = MeshLoader()(*cubeDetails, "Fallback Cube");
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

		mQueue.emplace_back(ResourceLoadDetails_Internal{ id,  copy, debugName });

		return id;
	}

	void MeshManager::_tickImpl() {
		TRACY_ZONE();

		if (!mQueue.empty()) {
			std::vector<ResourceLoadDetails_Internal> swapQueue = {};
			std::swap(mQueue, swapQueue);
			mQueue.clear();

			TRACY_GPUN("Create");
			for (auto& details : swapQueue) {
				TRACY_GPUN("Create Single");
				mCache.load(details.mHandle.mHandle, details.mLoadDetails, details.mDebugName);
				for (auto&& [type, buffer] : details.mLoadDetails.mVertexBuffers) {
					free(const_cast<uint8_t*>(buffer.mData));
				}
				if (details.mLoadDetails.mElementBuffer.has_value()) {
					free(const_cast<uint8_t*>(details.mLoadDetails.mElementBuffer->mData));
				}
			}
		}

		if (!mTransactionQueue.empty()) {
			std::vector<std::pair<MeshHandle, std::function<void(Mesh&)>>> swapQueue;
			std::swap(mTransactionQueue, swapQueue);
			mTransactionQueue.clear();

			TRACY_GPUN("Transact");
			for (auto&& [handle, func] : swapQueue) {
				TRACY_GPUN("Transact Single");
				if (isValid(handle)) {
					func(mCache[handle.mHandle]->mResource);
				}
				else {
					NEO_LOG_E("Attempting to transact on an invalid mesh");
				}
			}
		}

		if (!mDiscardQueue.empty()) {
			std::vector<MeshHandle> swapQueue;
			std::swap(mDiscardQueue, swapQueue);
			mDiscardQueue.clear();

			TRACY_GPUN("Destroy");
			for (auto& id : swapQueue) {
				TRACY_GPUN("Destroy Single");
				if (isValid(id)) {
					_destroyImpl(mCache[id.mHandle]);
					mCache.erase(id.mHandle);
				}
			}
		}
	}

	void MeshManager::_destroyImpl(BackedResource<Mesh>& mesh) {
		mesh.mResource.destroy();
	}

	void MeshManager::imguiEditor(ECS& ecs) {
		for(auto&& [handle, meshResource] : mCache) {
			ImGui::PushID(static_cast<int>(handle));
			bool node = false;
			if (meshResource->mDebugName.has_value()) {
				node |= ImGui::TreeNode(static_cast<void*>(&handle), "%s", meshResource->mDebugName->c_str());
			}
			else {
				node |= ImGui::TreeNode(static_cast<void*>(&handle), "%d", handle);
			}
			if (node) {
				ImGui::InvisibleButton("Show mesh", ImVec2(175.f, 175.f));
				ImGuiMeshViewComponent component;
				component.mMeshHandle = handle;
				component.mBounds.x = static_cast<uint16_t>(ImGui::GetItemRectMin().x);
				component.mBounds.y = static_cast<uint16_t>(ImGui::GetItemRectMin().y);
				component.mBounds.z = static_cast<uint16_t>(ImGui::GetItemRectMax().x);
				component.mBounds.w = static_cast<uint16_t>(ImGui::GetItemRectMax().y);
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<ImGuiComponent>()
					.attachComponent<ImGuiMeshViewComponent>(std::move(component))
				));
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	MeshLoader::result_type MeshLoader::operator()(MeshLoadDetails meshDetails, const std::optional<std::string>& debugName) const {
		if (debugName.has_value()) {
			NEO_LOG_V("Uploading mesh %s", debugName.value().c_str());
		}
		result_type meshResource = std::make_shared<BackedResource<Mesh>>(meshDetails.mPrimtive);
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
