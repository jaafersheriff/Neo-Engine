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
            Mesh() :
                mVAOID(0),
                mVertexBufferID(0),
                mNormalBufferID(0),
                mUVBufferID(0),
                mElementBufferID(0),
                mVertexBufferSize(0),
                mNormalBufferSize(0),
                mUVBufferSize(0),
                mElementBufferSize(0),
                mPrimitiveType(0)
            {}

            Mesh(MeshBuffers& buffers) 
                : Mesh() {
                mVertexBufferSize = buffers.vertices.size();
                mNormalBufferSize = buffers.normals.size();
                mUVBufferSize = buffers.texCoords.size();
                mElementBufferSize = buffers.indices.size();
                mBuffers = buffers;
            }

            /* Destructor */
            ~Mesh();

            /* VAO ID */
            unsigned int mVAOID;

            /* VBO IDs */
            unsigned int mVertexBufferID;
            unsigned int mNormalBufferID;
            unsigned int mUVBufferID;
            unsigned int mElementBufferID;

            /* VBO Info */
            MeshBuffers mBuffers;
            int mVertexBufferSize;
            int mNormalBufferSize;
            int mUVBufferSize;
            int mElementBufferSize;

            /* Primitive type */
            unsigned mPrimitiveType;

            /* Copy data to GPU */
            void upload(unsigned = 0);

            /* Call the appropriate draw function */
            void draw(unsigned = 0) const;
    };
}