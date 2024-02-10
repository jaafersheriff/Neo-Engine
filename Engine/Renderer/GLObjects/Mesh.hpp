#pragma once

#include <optional>
#include <unordered_map>

namespace neo {

	enum class VertexType : uint8_t {
		Position = 0,
		Normal,
		Texture0,
		Texture1,
		Texture2,
		Color0,
		Color1,
		Color2,
		COUNT
	};

	struct VertexBuffer {
		uint32_t vboID = 0;
		uint32_t attribArray = 0;
		uint32_t stride = 3;
		uint32_t elementCount = 0;
		uint32_t format = 0;
	};

	class Mesh {

		public:

			/* Constructor */
			Mesh(int primitiveType = -1);
			~Mesh();

			/* VAO ID */
			uint32_t mVAOID;

			/* VBOs */
			void addVertexBuffer_DEPRECATED(VertexType type, uint32_t attribArray, uint32_t stride, const std::vector<float>& buffer = {});
			void addVertexBuffer(VertexType type, uint32_t components, uint32_t stride, uint32_t format, bool normalized, uint32_t count, uint32_t offset, uint32_t byteSize, const uint8_t* data = nullptr);
			void updateVertexBuffer_DEPRECATED(VertexType type, const std::vector<float>& buffer);
			void updateVertexBuffer_DEPRECATED(VertexType type, uint32_t size);
			void removeVertexBuffer(VertexType type);

			void addElementBuffer_DEPRECATED(const std::vector<uint32_t>& buffer = {});
			void addElementBuffer(uint32_t count, uint32_t format, uint32_t byteSize, const uint8_t* data = nullptr);
			void updateElementBuffer_DEPRECATED(const std::vector<uint32_t>& buffer);
			void updateElementBuffer_DEPRECATED(uint32_t size);
			void removeElementBuffer();

			const VertexBuffer& getVBO(VertexType type) const;

			/* Primitive type */
			uint32_t mPrimitiveType;

			/* Call the appropriate draw function */
			void draw(uint32_t = 0) const;

			/* Remove */
			void clear();
			void destroy();

		private:
			std::unordered_map<VertexType, VertexBuffer> mVBOs;
			std::optional<VertexBuffer> mElementVBO;
			
	};
}
