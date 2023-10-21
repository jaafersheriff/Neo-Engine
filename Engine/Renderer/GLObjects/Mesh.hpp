#pragma once

#include <optional>
#include <unordered_map>

namespace neo {

    enum class VertexType {
        Position,
        Normal,
        Texture0,
        Texture1,
        Texture2,
        Color0,
        Color1,
        Color2
    };

    struct VertexBuffer {
        uint32_t vboID = 0;
        uint32_t attribArray = 0;
        uint32_t stride = 3;
        uint32_t bufferSize = 0;
    };

    class Mesh {

        public:

            /* Constructor */
            Mesh(int primitiveType = -1);
            ~Mesh();

            /* VAO ID */
            uint32_t mVAOID;

            /* VBOs */
            void addVertexBuffer(VertexType type, uint32_t attribArray, uint32_t stride, const std::vector<float>& buffer = {});
            void updateVertexBuffer(VertexType type, const std::vector<float>& buffer);
            void updateVertexBuffer(VertexType type, uint32_t size);
            void removeVertexBuffer(VertexType type);

            void addElementBuffer(const std::vector<uint32_t>& buffer = {});
            void updateElementBuffer(const std::vector<uint32_t>& buffer);
            void updateElementBuffer(uint32_t size);
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
