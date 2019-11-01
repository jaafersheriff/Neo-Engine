#include "Mesh.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    Mesh::Mesh(unsigned primitiveType) :
        mVAOID(0),
        mVertexBufferID(0),
        mNormalBufferID(0),
        mTexBufferID(0),
        mElementBufferID(0),
        mVertexBufferSize(0),
        mNormalBufferSize(0),
        mTexBufferSize(0),
        mElementBufferSize(0),
        mPrimitiveType(primitiveType)
    {
        /* Initialize VAO */
        CHECK_GL(glGenVertexArrays(1, (GLuint *)&mVAOID));
    }

    Mesh::Mesh(MeshBuffers& buffers)
        : Mesh() {
        mBuffers = buffers;

        if (!mPrimitiveType) {
            if (mElementBufferSize) {
                mPrimitiveType = GL_TRIANGLES;
            }
            else {
                mPrimitiveType = GL_TRIANGLE_STRIP;
            }
        }

        upload();
    }



    void Mesh::draw(unsigned size) const {
        if (mElementBufferSize) {
            // TODO - instanced?
            CHECK_GL(glDrawElements(mPrimitiveType, size ? size : mElementBufferSize, GL_UNSIGNED_INT, nullptr));
        }
        else if (size) {
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, size));
        }
        else {
            int vSize = 0;
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
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, vSize));
        }
    }

    void Mesh::upload(int type) {
        _upload();

        /* Set draw mode */
        if (type > -1) {
            mPrimitiveType = type;
        }

        mVertexBufferSize = mBuffers.vertices.size();
        mNormalBufferSize = mBuffers.normals.size();
        mTexBufferSize = mBuffers.texCoords.size();
        mElementBufferSize = mBuffers.indices.size();

    }

    void Mesh::_upload() {
        CHECK_GL(glBindVertexArray(mVAOID));

        /* Copy vertex array */
        if (!mVertexBufferID) {
            CHECK_GL(glGenBuffers(1, (GLuint *)&mVertexBufferID));
        }
        if (mBuffers.vertices.size() && mVertexBufferID) {
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.vertices.size() * sizeof(float), &mBuffers.vertices[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID));
            CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy normal array if it exists */
        if (!mNormalBufferID) {
            CHECK_GL(glGenBuffers(1, (GLuint *)&mNormalBufferID));
        }
        if (!mBuffers.normals.empty() && mNormalBufferID) {
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mNormalBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.normals.size() * sizeof(float), &mBuffers.normals[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(1));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mNormalBufferID));
            CHECK_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy texture array if it exists */
        if (!mTexBufferID) {
            CHECK_GL(glGenBuffers(1, (GLuint *)&mTexBufferID));
        }
        if (!mBuffers.texCoords.empty() && mTexBufferID) {
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mTexBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mBuffers.texCoords.size() * sizeof(float), &mBuffers.texCoords[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(2));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mTexBufferID));
            CHECK_GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy element array if it exists */
        if (!mBuffers.indices.empty()) {
            if (!mElementBufferID) {
                CHECK_GL(glGenBuffers(1, (GLuint *)&mElementBufferID));
            }
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferID));
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mBuffers.indices.size() * sizeof(unsigned int), &mBuffers.indices[0], GL_STATIC_DRAW));
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