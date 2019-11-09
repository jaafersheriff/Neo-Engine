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
            void addVertexBuffer(VertexType type, unsigned attribArray, unsigned stride, const std::vector<float>& buffer = {});
            void updateVertexBuffer(VertexType type, const std::vector<float>& buffer);
            void updateVertexBuffer(VertexType type, unsigned size);
            void removeVertexBuffer(VertexType type);

            void addElementBuffer(const std::vector<unsigned>& buffer = {});
            void updateElementBuffer(const std::vector<unsigned>& buffer);
            void updateElementBuffer(unsigned size);
            void removeElementBuffer();

            const VertexBuffer& getVBO(VertexType type);

            /* Primitive type */
            unsigned mPrimitiveType;

            /* Call the appropriate draw function */
            void draw(unsigned = 0) const;

            /* Remove */
            void destroy();

        private:
            std::unordered_map<VertexType, VertexBuffer> mVBOs;
            
    public://todo - hax

            std::optional<VertexBuffer> mElementVBO;
    };
}