#include "Mesh.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    Mesh::Mesh(int primitiveType) :
        mVAOID(0),
        mPrimitiveType(primitiveType)
    {
        /* Initialize VAO */
        CHECK_GL(glGenVertexArrays(1, (GLuint *)&mVAOID));
    }

    void Mesh::draw(unsigned size) const {
        CHECK_GL(glBindVertexArray(mVAOID));
        if (mElementVBO) {
            // TODO - instanced?
            CHECK_GL(glDrawElements(mPrimitiveType, size ? size : mElementVBO->bufferSize, GL_UNSIGNED_INT, nullptr));
        }
        else if (size) {
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, size));
        }
        else {
            const auto& positions = mVBOs.find(VertexType::Position);
            assert(positions != mVBOs.end());
            int vSize = 0;
            switch (mPrimitiveType) {
                case GL_POINTS:
                    vSize = positions->second.bufferSize / 3;
                case GL_LINE_STRIP:
                    vSize = positions->second.bufferSize / 3 - 1;
                    break;
                case GL_TRIANGLE_STRIP:
                    vSize = positions->second.bufferSize / 4 - 3;
                    break;
                case GL_LINES:
                    vSize = positions->second.bufferSize / 6;
                    break;
                case GL_TRIANGLES:
                    vSize = positions->second.bufferSize / 9;
                    break;
                default:
                    break;
            }
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, positions->second.bufferSize));
        }
    }

    void Mesh::addVertexBuffer(VertexType type, unsigned attribArray, unsigned stride, const std::vector<float>& buffer) {
        {
            const auto& vbo = mVBOs.find(type);
            assert(vbo == mVBOs.end());
        }

        mVBOs[type] = {};
        auto& vertexBuffer = mVBOs[type];
        vertexBuffer.attribArray = attribArray;
        vertexBuffer.stride = stride;
        vertexBuffer.bufferSize = buffer.size();

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glGenBuffers(1, (GLuint *)&vertexBuffer.vboID));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID));
        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW));
        }
        CHECK_GL(glEnableVertexAttribArray(attribArray));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID));
        CHECK_GL(glVertexAttribPointer(attribArray, stride, GL_FLOAT, GL_FALSE, 0, (const void *)0));

        assert(glGetError() == GL_NO_ERROR);
    }

    void Mesh::updateVertexBuffer(VertexType type, const std::vector<float>& buffer) {
        const auto& vbo = mVBOs.find(type);
        assert(vbo != mVBOs.end());
        auto& vertexBuffer = vbo->second;
        vertexBuffer.bufferSize = buffer.size();

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer.vboID));
        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW));
        }

        assert(glGetError() == GL_NO_ERROR);
    }

    void Mesh::updateVertexBuffer(VertexType type, unsigned size) {
        assert(size > 0);
        const auto& vbo = mVBOs.find(type);
        assert(vbo != mVBOs.end());
        auto& vertexBuffer = vbo->second;
        vertexBuffer.bufferSize = size;

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer.vboID));
        CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), (const void *)0, GL_STATIC_DRAW));

        assert(glGetError() == GL_NO_ERROR);
    }

    void Mesh::removeVertexBuffer(VertexType type) {
        const auto& vbo = mVBOs.find(type);
        if (vbo != mVBOs.end()) {
            CHECK_GL(glBindVertexArray(mVAOID));
            CHECK_GL(glDeleteBuffers(1, (GLuint *)&vbo->second.vboID));
            mVBOs.erase(type);
        }
    }

    const VertexBuffer& Mesh::getVBO(VertexType type) {
        auto vbo = mVBOs.find(type);
        assert(vbo != mVBOs.end());
        return vbo->second;
    }

    void Mesh::addElementBuffer(const std::vector<unsigned>& buffer) {
        assert(!mElementVBO.has_value());
        mElementVBO = std::make_optional<VertexBuffer>();
        mElementVBO->bufferSize = buffer.size();

        CHECK_GL(glBindVertexArray(mVAOID));

        CHECK_GL(glGenBuffers(1, (GLuint *)&mElementVBO->vboID));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID));
        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(unsigned int), buffer.data(), GL_STATIC_DRAW));
        }

        assert(glGetError() == GL_NO_ERROR);

        mPrimitiveType = GL_TRIANGLES;
    }

    void Mesh::updateElementBuffer(const std::vector<unsigned>& buffer) {
        assert(mElementVBO.has_value() && buffer.size());
        mElementVBO->bufferSize = buffer.size();

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID));
        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(unsigned), buffer.data(), GL_STATIC_DRAW));
        }

        assert(glGetError() == GL_NO_ERROR);
    }

    void Mesh::updateElementBuffer(unsigned size) {
        assert(mElementVBO.has_value() && size > 0);
        mElementVBO->bufferSize = size;

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID));
        CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(unsigned), (const void *)0, GL_STATIC_DRAW));

        assert(glGetError() == GL_NO_ERROR);
    }

    void Mesh::removeElementBuffer() {
        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glDeleteBuffers(1, (GLuint *)&mElementVBO->vboID));
        mElementVBO = std::nullopt;

        mPrimitiveType = GL_TRIANGLE_STRIP;
    }

    void Mesh::destroy() {
        removeVertexBuffer(VertexType::Position);
        removeVertexBuffer(VertexType::Normal);
        removeVertexBuffer(VertexType::Texture0);
        removeVertexBuffer(VertexType::Texture1);
        removeVertexBuffer(VertexType::Texture2);
        removeVertexBuffer(VertexType::Color0);
        removeVertexBuffer(VertexType::Color1);
        removeVertexBuffer(VertexType::Color2);
    }
}