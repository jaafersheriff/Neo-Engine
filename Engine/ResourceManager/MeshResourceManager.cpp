#include "MeshResourceManager.hpp"

namespace neo {

	struct MeshLoader final : entt::resource_loader<MeshLoader, Mesh> {

		std::shared_ptr<Mesh> load(MeshManager::MeshBuilder meshDetails) const {
			std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(meshDetails.mPrimtive);
			mesh->init();
			for (auto&& [type, buffer] : meshDetails.mVertexBuffers) {
				mesh->addVertexBuffer(
					type,
					buffer.components,
					buffer.stride,
					buffer.format,
					buffer.normalized,
					buffer.count,
					buffer.offset,
					buffer.byteSize,
					buffer.data
				);
			}
			if (meshDetails.mElementBuffer) {
				mesh->addElementBuffer(
					meshDetails.mElementBuffer->count,
					meshDetails.mElementBuffer->format,
					meshDetails.mElementBuffer->byteSize,
					meshDetails.mElementBuffer->data
				);
			}
			mesh->mMin = meshDetails.mMin;
			mesh->mMax = meshDetails.mMax;
			return mesh;
		}
	};

	Mesh& MeshManager::get(HashedString id) {
		return get(id.value());
	}

	const Mesh& MeshManager::get(HashedString id) const {
		return get(id.value());
	}

	Mesh& MeshManager::get(MeshHandle id) {
		if (mMeshCache.contains(id)) {
			return mMeshCache.handle(id).get();
		}
		NEO_LOG_E("Invalid mesh requested");
		return mMeshCache.handle(HashedString("cube")).get();
	}

	const Mesh& MeshManager::get(MeshHandle id) const {
		if (mMeshCache.contains(id)) {
			return mMeshCache.handle(id).get();
		}
		NEO_LOG_E("Invalid mesh requested");
		return mMeshCache.handle(HashedString("cube")).get();
	}

	[[nodiscard]] MeshHandle MeshManager::load(HashedString id, MeshBuilder meshDetails) {
		mMeshCache.load<MeshLoader>(id, meshDetails);
		return id;
	}

	void MeshManager::clear() {
		mMeshCache.each([](Mesh& mesh) {
			mesh.destroy();
			});
		mMeshCache.clear();
	}
}