#pragma once

#include "Renderer/GLObjects/Mesh.hpp"

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>

#include <memory>

namespace neo {

	struct MeshLoader final: entt::resource_loader<MeshLoader, Mesh> {
		struct MeshBuilder {
			types::mesh::Primitive mPrimtive;
			struct VertexBuffer {
				uint32_t components; 
				uint32_t stride; 
				types::ByteFormats format; 
				bool normalized; 
				uint32_t count; 
				uint32_t offset; 
				uint32_t byteSize; 
				const uint8_t* data = nullptr;
			};
			struct ElementBuffer {
				uint32_t count;
				types::ByteFormats format;
				uint32_t byteSize;
				const uint8_t* data = nullptr;
			};
			std::unordered_map<types::mesh::VertexType, VertexBuffer> mVertexBuffers;
			std::optional<ElementBuffer> mElementBuffer;
			glm::vec3 mMin = glm::vec3(0.f);
			glm::vec3 mMax = glm::vec3(0.f);
		};

		std::shared_ptr<Mesh> load(MeshBuilder meshDetails) const {
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

	using MeshCache = entt::resource_cache<Mesh>;
	using MeshHandle = entt::id_type;
	class MeshManager {
	public:
		MeshCache meshCache;

		Mesh& get(HashedString id) {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh %s requested", id.data());
			return meshCache.handle(HashedString("cube")).get();
		}

		const Mesh& get(HashedString id) const {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh %s requested", id.data());
			return meshCache.handle(HashedString("cube")).get();
		}

		Mesh& get(MeshHandle id) {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh requested");
			return meshCache.handle(HashedString("cube")).get();
		}

		const Mesh& get(MeshHandle id) const {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh requested");
			return meshCache.handle(HashedString("cube")).get();
		}

		[[nodiscard]] MeshHandle load(HashedString id, MeshLoader::MeshBuilder meshDetails) {
			meshCache.load<MeshLoader>(id, meshDetails);
			return id;
		}

		void clear() {
			meshCache.each([](Mesh& mesh) {
				mesh.destroy();
			});
			meshCache.clear();
		}
		void imguiEditor() {
			meshCache.each([](const entt::id_type id) {
				NEO_UNUSED(id);
				// ImGuiText
			});
		}
	private:
	};
}