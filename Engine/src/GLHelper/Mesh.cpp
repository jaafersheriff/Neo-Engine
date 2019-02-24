#include "Mesh.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLHelper/GLHelper.hpp"

namespace neo {

    Mesh::~Mesh() {
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mVertexBufferID));
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mNormalBufferID));
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mTextureBufferID));
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mElementBufferID));
        CHECK_GL(glDeleteVertexArrays(1, (GLuint *) &mVAOID));
    }

    void Mesh::draw(unsigned size) const {
        if (mElementBufferSize) {
            // TODO - instanced?
            CHECK_GL(glDrawElements(mPrimitiveType, size ? size : mElementBufferSize, GL_UNSIGNED_INT, nullptr));
        }
        else {
            int vSize = size;
            if (!vSize) {
                switch (mPrimitiveType) {
                case GL_POINTS:
                case GL_LINE_STRIP:
                    vSize = mVertexBufferSize / 3;
                case GL_LINES:
                    vSize = mVertexBufferSize / 6;
                case GL_TRIANGLES:
                    vSize = mVertexBufferSize / 9;
                default:
                    vSize = mVertexBufferSize;
                }
            }
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, vSize));
        }
    }

    void Mesh::upload(unsigned type) {
        /* Initialize VAO */
        CHECK_GL(glGenVertexArrays(1, (GLuint *) &mVAOID));
        CHECK_GL(glBindVertexArray(mVAOID));

        /* Copy vertex array */
        CHECK_GL(glGenBuffers(1, (GLuint *) &mVertexBufferID));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID));
        if (mBuffers.vertBuf.size()) {
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.vertBuf.size() * sizeof(float), &mBuffers.vertBuf[0], GL_STATIC_DRAW));
        }
        CHECK_GL(glEnableVertexAttribArray(0));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID));
        CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

        /* Copy normal array if it exists */
        if (!mBuffers.norBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &mNormalBufferID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mNormalBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.norBuf.size() * sizeof(float), &mBuffers.norBuf[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(1));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mNormalBufferID));
            CHECK_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy texture array if it exists */
        if (!mBuffers.texBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &mTextureBufferID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mTextureBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.texBuf.size() * sizeof(float), &mBuffers.texBuf[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(2));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mTextureBufferID));
            CHECK_GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy element array if it exists */
        if (!mBuffers.eleBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &mElementBufferID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferID));
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mBuffers.eleBuf.size() * sizeof(unsigned int), &mBuffers.eleBuf[0], GL_STATIC_DRAW));
        }

        /* Set draw mode */
        if (type) {
            mPrimitiveType = type;
        }
        else if (!mBuffers.eleBuf.empty()) {
            mPrimitiveType = GL_TRIANGLES;
        }
        else {
            mPrimitiveType = GL_TRIANGLE_STRIP;
        }

        /* Unbind  */
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        /* Error check */
        assert(glGetError() == GL_NO_ERROR);
    }
}