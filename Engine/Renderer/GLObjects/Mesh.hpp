#pragma once

#include "Renderer/Types.hpp"

#include <optional>
#include <unordered_map>
#include <glm/glm.hpp>

namespace neo {

	struct VertexBuffer {
		uint32_t vboID = 0;
		uint32_t attribArray = 0;
		uint32_t components = 3;
		uint32_t stride = 0;
		uint32_t elementCount = 0;
		uint32_t format = 0;
	};

	class Mesh {

		public:
			Mesh(types::mesh::Primitive primitive = types::mesh::Primitive::TriangleStrip);

			/* VAO ID */
			uint32_t mVAOID;

			/* VBOs */
			void addVertexBuffer(types::mesh::VertexType type, uint32_t components, uint32_t stride, types::ByteFormats format, bool normalized, uint32_t count, uint32_t offset, uint32_t byteSize, const uint8_t* data = nullptr);
			void updateVertexBuffer(types::mesh::VertexType type, uint32_t count, uint32_t byteSize, uint8_t* data);
			void removeVertexBuffer(types::mesh::VertexType type);

			void addElementBuffer(uint32_t count, types::ByteFormats format, uint32_t byteSize, const uint8_t* data = nullptr);
			void removeElementBuffer();

			bool hasVBO(types::mesh::VertexType type) const;
			const VertexBuffer& getVBO(types::mesh::VertexType type) const;

			types::mesh::Primitive mPrimitiveType = types::mesh::Primitive::TriangleStrip;

			glm::vec3 mMin = glm::vec3(0.f);
			glm::vec3 mMax = glm::vec3(0.f);

			void draw(uint32_t = 0) const;

			void init();
			void destroy();
			void clear();

		private:
			std::unordered_map<types::mesh::VertexType, VertexBuffer> mVBOs;
			std::optional<VertexBuffer> mElementVBO;
			
	};
}
