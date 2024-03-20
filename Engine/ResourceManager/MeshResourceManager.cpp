#include "MeshResourceManager.hpp"

#include "Util/Profiler.hpp"

namespace neo {

	struct MeshLoader final : entt::resource_loader<MeshLoader, Mesh> {

		std::shared_ptr<Mesh> load(MeshResourceManager::MeshBuilder meshDetails) const {
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
			return mesh;
		}
	};

	bool MeshResourceManager::isValid(MeshHandle id) const {
		return mMeshCache.contains(id);
	}

	Mesh& MeshResourceManager::get(HashedString id) {
		return get(id.value());
	}

	const Mesh& MeshResourceManager::get(HashedString id) const {
		return get(id.value());
	}

	Mesh& MeshResourceManager::get(MeshHandle id) {
		if (mMeshCache.contains(id)) {
			return mMeshCache.handle(id).get();
		}
		NEO_FAIL("Invalid mesh requested");
		return mMeshCache.handle(HashedString("cube")).get();
	}

	const Mesh& MeshResourceManager::get(MeshHandle id) const {
		if (mMeshCache.contains(id)) {
			return mMeshCache.handle(id).get();
		}
		NEO_FAIL("Invalid mesh requested");
		return mMeshCache.handle(HashedString("cube")).get();
	}

	[[nodiscard]] MeshHandle MeshResourceManager::asyncLoad(HashedString id, MeshBuilder& meshDetails) const {
		if (!isValid(id)) {
			// Copy data so this can be ticked next frame
			MeshBuilder copy = meshDetails;
			for (auto&& [type, buffer] : meshDetails.mVertexBuffers) {
				if (buffer.data) {
					copy.mVertexBuffers[type].data = static_cast<uint8_t*>(malloc(buffer.byteSize));
					memcpy(const_cast<uint8_t*>(copy.mVertexBuffers[type].data), buffer.data, buffer.byteSize);
				}
			}
			if (meshDetails.mElementBuffer.has_value() && meshDetails.mElementBuffer->data) {
				copy.mElementBuffer->data = static_cast<uint8_t*>(malloc(meshDetails.mElementBuffer->byteSize));
				memcpy(const_cast<uint8_t*>(copy.mElementBuffer->data), meshDetails.mElementBuffer->data, meshDetails.mElementBuffer->byteSize);
			}

			mQueue.emplace_back(std::make_pair(id, copy));
		}

		return id;
	}

	void MeshResourceManager::_tick() {
		TRACY_ZONE();
		for (auto&& [id, meshDetails] : mQueue) {
			mMeshCache.load<MeshLoader>(id, meshDetails);
			for (auto&& [type, buffer] : meshDetails.mVertexBuffers) {
				free(const_cast<uint8_t*>(buffer.data));
			}
			if (meshDetails.mElementBuffer.has_value()) {
				free(const_cast<uint8_t*>(meshDetails.mElementBuffer->data));
			}
		}
		mQueue.clear();
	}

	void MeshResourceManager::clear() {
		mMeshCache.each([](Mesh& mesh) {
			mesh.destroy();
		});
		mMeshCache.clear();
	}
}