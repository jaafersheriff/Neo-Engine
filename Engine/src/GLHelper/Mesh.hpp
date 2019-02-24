#pragma once

#include "glm/glm.hpp"

#include <vector>

namespace neo {

    class Mesh {

        public:
            struct MeshBuffers {
                std::vector<float> vertBuf;
                std::vector<float> norBuf;
                std::vector<float> texBuf;
                std::vector<unsigned int> eleBuf;
            };

            /* Constructor */
            Mesh() :
                mVAOID(0),
                mVertexBufferID(0),
                mNormalBufferID(0),
                mTextureBufferID(0),
                mElementBufferID(0),
                mVertexBufferSize(0),
                mNormalBufferSize(0),
                mTextureBufferSize(0),
                mElementBufferSize(0),
                mPrimitiveType(0)
            {}

            /* Destructor */
            ~Mesh();

            /* VAO ID */
            unsigned int mVAOID;

            /* VBO IDs */
            unsigned int mVertexBufferID;
            unsigned int mNormalBufferID;
            unsigned int mTextureBufferID;
            unsigned int mElementBufferID;

            /* VBO Info */
            MeshBuffers mBuffers;
            int mVertexBufferSize;
            int mNormalBufferSize;
            int mTextureBufferSize;
            int mElementBufferSize;

            /* Primitive type */
            unsigned mPrimitiveType;

            /* Copy data to GPU */
            void upload(unsigned = 0);

            /* Call the appropriate draw function */
            void draw(unsigned = 0) const;
    };
}