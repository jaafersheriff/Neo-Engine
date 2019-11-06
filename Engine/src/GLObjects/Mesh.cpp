#include "Mesh.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    Mesh::Mesh(int primitiveType) :
        mVAOID(0),
        mVertexBufferID(0),
        mNormalBufferID(0),
        mTexBufferID(0),
        mElementBufferID(0),
        mPrimitiveType(primitiveType)
    {
        /* Initialize VAO */
        CHECK_GL(glGenVertexArrays(1, (GLuint *)&mVAOID));
    }

    Mesh::Mesh(MeshBuffers& buffers, int primitiveType)
        : Mesh(primitiveType) {

        mBuffers = buffers;
        upload();
    }

    void Mesh::draw(unsigned size) const {
        if (mElementBufferID) {
            // TODO - instanced?
            CHECK_GL(glDrawElements(mPrimitiveType, size ? size : mBuffers.indices.size(), GL_UNSIGNED_INT, nullptr));
        }
        else if (size) {
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, size));
        }
        else {
            int vSize = 0;
            switch (mPrimitiveType) {
                case GL_POINTS:
                    vSize = mBuffers.vertices.size() / 3;
                case GL_LINE_STRIP:
                    vSize = mBuffers.vertices.size() / 3 - 1;
                    break;
                case GL_TRIANGLE_STRIP:
                    vSize = mBuffers.vertices.size() / 4 - 3;
                    break;
                case GL_LINES:
                    vSize = mBuffers.vertices.size() / 6;
                    break;
                case GL_TRIANGLES:
                    vSize = mBuffers.vertices.size() / 9;
                    break;
                default:
                    break;
            }
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, vSize));
        }
    }

    void Mesh::upload() {
        _upload();

        /* Set draw mode */
        if (mPrimitiveType == -1) {
            if (mElementBufferID) {
                mPrimitiveType = GL_TRIANGLES;
            }
            else {
                mPrimitiveType = GL_TRIANGLE_STRIP;
            }
        }
    }

    void Mesh::_upload() {
        CHECK_GL(glBindVertexArray(mVAOID));

        /* Copy vertex array */
        if (!mBuffers.vertices.empty()) {
            if (!mVertexBufferID) {
                CHECK_GL(glGenBuffers(1, (GLuint *)&mVertexBufferID));
            }
            CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mVertexBufferID));
            CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, mBuffers.vertices.size() * sizeof(float), &mBuffers.vertices[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID));
            CHECK_GL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }
        else if (mVertexBufferID) {
            CHECK_GL(glDeleteBuffers(1, (GLuint *)&mVertexBufferID));
            mVertexBufferID = 0;
        }

        /* Copy normal array if it exists */
        if (!mBuffers.normals.empty()) {
            if (!mNormalBufferID) {
                CHECK_GL(glGenBuffers(1, (GLuint *)&mNormalBufferID));
            }
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mNormalBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.normals.size() * sizeof(float), &mBuffers.normals[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(1));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mNormalBufferID));
            CHECK_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }
        else if (mNormalBufferID) {
            CHECK_GL(glDeleteBuffers(1, (GLuint *)&mNormalBufferID));
            mNormalBufferID = 0;
        }

        /* Copy texture array if it exists */
        if (!mBuffers.texCoords.empty()) {
            if (!mTexBufferID) {
                CHECK_GL(glGenBuffers(1, (GLuint *)&mTexBufferID));
            }
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mTexBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.texCoords.size() * sizeof(float), &mBuffers.texCoords[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(2));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mTexBufferID));
            CHECK_GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }
        else if (mTexBufferID) {
            CHECK_GL(glDeleteBuffers(1, (GLuint *)&mTexBufferID));
            mTexBufferID = 0;
        }

        /* Copy element array if it exists */
        if (!mBuffers.indices.empty()) {
            if (!mElementBufferID) {
                CHECK_GL(glGenBuffers(1, (GLuint *)&mElementBufferID));
            }
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferID));
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mBuffers.indices.size() * sizeof(unsigned int), &mBuffers.indices[0], GL_STATIC_DRAW));
        }
        else if (mElementBufferID) {
            CHECK_GL(glDeleteBuffers(1, (GLuint *)&mElementBufferID));
            mElementBufferID = 0;
        }

        /* Unbind  */
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        /* Error check */
        assert(glGetError() == GL_NO_ERROR);
    }

    void Mesh::destroy() {
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mVertexBufferID));
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mNormalBufferID));
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mTexBufferID));
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mElementBufferID));
        CHECK_GL(glDeleteVertexArrays(1, (GLuint *) &mVAOID));

    }
}