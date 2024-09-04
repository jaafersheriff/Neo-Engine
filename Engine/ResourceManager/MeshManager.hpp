#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

namespace neo {
	class ResourceManagers;

	struct MeshLoadDetails {
		types::mesh::Primitive mPrimtive;

		struct VertexBuffer {
			uint32_t mComponents; 
			uint32_t mStride; 
			types::ByteFormats mFormat; 
			bool mNormalized; 
			uint32_t mCount; 
			uint32_t mOffset; 
			uint32_t mByteSize; 
			const uint8_t* mData = nullptr;
		};
		struct ElementBuffer {
			uint32_t mCount;
			types::ByteFormats mFormat;
			uint32_t mByteSize;
			const uint8_t* mData = nullptr;
		};
		std::unordered_map<types::mesh::VertexType, VertexBuffer> mVertexBuffers;
		std::optional<ElementBuffer> mElementBuffer;
	};
	using MeshHandle = ResourceHandle<Mesh>;

	struct MeshLoader final {
		using result_type = std::shared_ptr<BackedResource<Mesh>>;
		result_type operator()(MeshLoadDetails meshDetails, const std::optional<std::string>& debugName) const;
	};
	class MeshManager final : public ResourceManagerInterface<MeshManager, Mesh, MeshLoadDetails, MeshLoader> {
		friend ResourceManagerInterface;
	public:

		MeshManager();
		~MeshManager();

		void imguiEditor();

	protected:
		[[nodiscard]] MeshHandle _asyncLoadImpl(MeshHandle id, MeshLoadDetails meshDetails, const std::optional<std::string>& debugName) const;
		void _destroyImpl(BackedResource<Mesh>& mesh);
		void _tickImpl(RenderThread& renderThread);

	};
}