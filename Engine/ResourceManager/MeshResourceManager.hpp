#pragma once

#include "Renderer/GLObjects/Mesh.hpp"

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>

#include <memory>

namespace neo {

	using MeshHandle = entt::id_type;
	class MeshManager {
	public:
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

		using MeshCache = entt::resource_cache<Mesh>;
		MeshCache mMeshCache;

		Mesh& get(HashedString id);
		const Mesh& get(HashedString id) const;
		Mesh& get(MeshHandle id);
		const Mesh& get(MeshHandle id) const;

		[[nodiscard]] MeshHandle load(HashedString id, MeshBuilder meshDetails);

		void clear();
	};
}