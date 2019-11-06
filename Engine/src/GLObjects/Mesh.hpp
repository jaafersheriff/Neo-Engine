#pragma once

#include "glm/glm.hpp"

#include <vector>
#include <unordered_map>
#include <optional>

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
        unsigned vboID = 0;
        unsigned attribArray = 0;
        unsigned stride = 3;
        unsigned bufferSize = 0;
    };

    class Mesh {

        public:

            /* Constructor */
            Mesh(int primitiveType = -1);

            /* Remove copy constructors */
            Mesh(const Mesh &) = delete;
            Mesh & operator=(const Mesh &) = delete;
            Mesh(Mesh &&) = default;
            Mesh & operator=(Mesh &&) = default;

            /* Min/max */
            glm::vec3 mMin = glm::vec3(0.f);
            glm::vec3 mMax = glm::vec3(0.f);

            /* VAO ID */
            unsigned int mVAOID;

            /* VBOs */
            void addVertexBuffer(VertexType type, unsigned attribArray, unsigned stride, std::vector<float>* buffer = nullptr);
            void updateVertexBuffer(VertexType type, std::vector<float>& buffer);
            void removeVertexBuffer(VertexType type);
            std::unordered_map<VertexType, VertexBuffer> mVBOs;

            void addElementBuffer(std::vector<unsigned>& buffer);
            void removeElementBuffer();
            std::optional<VertexBuffer> mElementVBO;

            /* Primitive type */
            unsigned mPrimitiveType;

            /* Call the appropriate draw function */
            void draw(unsigned = 0) const;

            /* Remove */
            void destroy();
    };
}