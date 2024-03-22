#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>
#include <memory>

namespace neo {
	class ResourceManagers;

	struct MeshLoadDetails {
		types::mesh::Primitive mPrimtive;

		// TODO - add member naming convention
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
	};

	using MeshHandle = entt::id_type;
	class MeshResourceManager final : public ResourceManagerInterface<MeshResourceManager, MeshHandle, Mesh, MeshLoadDetails> {
		friend ResourceManagers;
		friend ResourceManagerInterface;
	public:
		MeshResourceManager();
		~MeshResourceManager();

	protected:
		[[nodiscard]] MeshHandle _asyncLoadImpl(HashedString id, MeshLoadDetails& meshDetails) const;
		void _clearImpl();
		void _tickImpl();
	};
}