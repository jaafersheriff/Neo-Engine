#include "MeshResourceManager.hpp"

#include "Loader/MeshGenerator.hpp"

#include "Util/Profiler.hpp"

namespace neo {

	struct MeshLoader final : entt::resource_loader<MeshLoader, Mesh> {

		std::shared_ptr<Mesh> load(MeshLoadDetails meshDetails) const {
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

	MeshResourceManager::MeshResourceManager() {
		auto cubeDetails = prefabs::generateCube();
		mFallback = MeshLoader{}.load(*cubeDetails);
		for (auto&& [type, buffer] : cubeDetails->mVertexBuffers) {
			free(const_cast<uint8_t*>(buffer.data));
		}
		if (cubeDetails->mElementBuffer) {
			free(const_cast<uint8_t*>(cubeDetails->mElementBuffer->data));
		}
	}

	MeshResourceManager::~MeshResourceManager() {
		mFallback.reset();
	}

	[[nodiscard]] MeshHandle MeshResourceManager::_asyncLoadImpl(HashedString id, MeshLoadDetails& meshDetails) const {
		if (!isValid(id) && mQueue.find(id) == mQueue.end()) {
			NEO_LOG_V("Loading mesh %s", id.data());

			// Copy data so this can be ticked next frame
			MeshLoadDetails copy = meshDetails;
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

			mQueue.emplace(id, copy);
		}

		return id;
	}

	void MeshResourceManager::_tickImpl() {
		TRACY_ZONE();

		std::map<MeshHandle, MeshLoadDetails> swapQueue = {};
		std::swap(mQueue, swapQueue);
		mQueue.clear();

		for (auto&& [id, meshDetails] : swapQueue) {
			mCache.load<MeshLoader>(id, meshDetails);
			for (auto&& [type, buffer] : meshDetails.mVertexBuffers) {
				free(const_cast<uint8_t*>(buffer.data));
			}
			if (meshDetails.mElementBuffer.has_value()) {
				free(const_cast<uint8_t*>(meshDetails.mElementBuffer->data));
			}
		}
	}

	void MeshResourceManager::_clearImpl() {
		mQueue.clear();
		mCache.each([](Mesh& mesh) {
			mesh.destroy();
		});
		mCache.clear();
	}
}