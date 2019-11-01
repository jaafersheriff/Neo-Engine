#pragma once

#include "glm/glm.hpp"

#include <vector>

namespace neo {

    class Mesh {

        public:
            struct MeshBuffers {
                std::vector<float> vertices;
                std::vector<float> normals;
                std::vector<float> texCoords;
                std::vector<unsigned int> indices;
            };

            /* Constructor */
            Mesh(unsigned primitiveType = 0);
            Mesh(MeshBuffers& buffers);

            /* Remove copy constructors */
            Mesh(const Mesh &) = delete;
            Mesh & operator=(const Mesh &) = delete;
            Mesh(Mesh &&) = default;
            Mesh & operator=(Mesh &&) = default;


            /* VAO ID */
            unsigned int mVAOID;

            /* VBO IDs */
            unsigned int mVertexBufferID;
            unsigned int mNormalBufferID;
            unsigned int mTexBufferID;
            unsigned int mElementBufferID;

            /* VBO Info */
            MeshBuffers mBuffers;
            int mVertexBufferSize;
            int mNormalBufferSize;
            int mTexBufferSize;
            int mElementBufferSize;

            /* Primitive type */
            unsigned mPrimitiveType;

            /* Copy data to GPU */
            void upload(int = -1);
            void reupload();

            /* Call the appropriate draw function */
            void draw(unsigned = 0) const;

            /* Remove */
            void destroy();

    private:
        void _upload();
    };
}