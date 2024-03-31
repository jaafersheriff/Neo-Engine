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
				mesh->addElementBuffer(
					meshDetails.mElementBuffer->mCount,
					meshDetails.mElementBuffer->mFormat,
					meshDetails.mElementBuffer->mByteSize,
					meshDetails.mElementBuffer->mData
				);
			}
			return mesh;
		}
	};

	MeshResourceManager::MeshResourceManager() {
		auto cubeDetails = prefabs::generateCube();
		mFallback = MeshLoader{}.load(*cubeDetails);
		for (auto&& [type, buffer] : cubeDetails->mVertexBuffers) {
			free(const_cast<uint8_t*>(buffer.mData));
		}
		if (cubeDetails->mElementBuffer) {
			free(const_cast<uint8_t*>(cubeDetails->mElementBuffer->mData));
		}
	}

	MeshResourceManager::~MeshResourceManager() {
		mFallback->destroy();
		mFallback.reset();
	}

	[[nodiscard]] MeshHandle MeshResourceManager::_asyncLoadImpl(MeshHandle id, MeshLoadDetails meshDetails, std::string debugName) const {
		NEO_LOG_V("Loading mesh %s", debugName.c_str());

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

		mQueue.emplace(id, ResourceLoadDetails_Internal{ copy, debugName });

		return id;
	}

	void MeshResourceManager::_tickImpl() {
		TRACY_ZONE();

		std::map<MeshHandle, ResourceLoadDetails_Internal> swapQueue = {};
		std::swap(mQueue, swapQueue);
		mQueue.clear();

		for (auto&& [handle, details] : swapQueue) {
			mCache.load<MeshLoader>(handle, details.mLoadDetails);
			for (auto&& [type, buffer] : details.mLoadDetails.mVertexBuffers) {
				free(const_cast<uint8_t*>(buffer.mData));
			}
			if (details.mLoadDetails.mElementBuffer.has_value()) {
				free(const_cast<uint8_t*>(details.mLoadDetails.mElementBuffer->mData));
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