#include "Mesh.hpp"
#include "Util/Util.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Renderer/Renderer.hpp"

#include "GL/glew.h"
#include "microprofile.h"

namespace neo {

    Mesh::Mesh(int primitiveType) :
        mVAOID(0),
        mPrimitiveType(primitiveType < 0 ? GL_TRIANGLE_STRIP : primitiveType)
    {
        /* Initialize VAO */
        CHECK_GL(glGenVertexArrays(1, (GLuint *)&mVAOID));
    }

    Mesh::~Mesh() {
        destroy();
    }

    // TODO - instanced
    void Mesh::draw(uint32_t size) const {
        Renderer::mStats.mNumDraws++;

        const auto& positions = getVBO(VertexType::Position);

        CHECK_GL(glBindVertexArray(mVAOID));
        if (mElementVBO) {
            CHECK_GL(glDrawElements(mPrimitiveType, size ? size : mElementVBO->bufferSize, GL_UNSIGNED_INT, nullptr));
        }
        else if (size) {
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, size));
        }
        else {
            CHECK_GL(glDrawArrays(mPrimitiveType, 0, positions.bufferSize / positions.stride));
        }
        CHECK_GL(glBindVertexArray(0));
    }

    void Mesh::addVertexBuffer(VertexType type, uint32_t attribArray, uint32_t stride, const std::vector<float>& buffer) {
        {
            const auto& vbo = mVBOs.find(type);
            NEO_ASSERT(vbo == mVBOs.end(), "Attempting to add a VertexBuffer that already exists");
        }

        auto vertexBuffer = VertexBuffer{};
        vertexBuffer.attribArray = attribArray;
        vertexBuffer.stride = stride;
        vertexBuffer.bufferSize = static_cast<uint32_t>(buffer.size());

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glGenBuffers(1, (GLuint *)&vertexBuffer.vboID));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID));

        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_STATIC_DRAW));
        }
        CHECK_GL(glEnableVertexAttribArray(attribArray));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID));
        CHECK_GL(glVertexAttribPointer(attribArray, stride, GL_FLOAT, GL_FALSE, 0, (const void *)0));

        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindVertexArray(0));

        mVBOs[type] = vertexBuffer;
    }

    void Mesh::updateVertexBuffer(VertexType type, const std::vector<float>& buffer) {
        MICROPROFILE_SCOPEI("Mesh", "updateVertexBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateVertexBuffer", MP_AUTO);

        const auto& vbo = mVBOs.find(type);
        NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
        auto& vertexBuffer = vbo->second;
        vertexBuffer.bufferSize = static_cast<uint32_t>(buffer.size());

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID));
        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_STATIC_DRAW));
        }
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindVertexArray(0));
    }

    void Mesh::updateVertexBuffer(VertexType type, uint32_t size) {
        MICROPROFILE_SCOPEI("Mesh", "updateVertexBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateVBO", MP_AUTO);

        NEO_ASSERT(size > 0, "Attempting to update a VertexBuffer with no data");
        const auto& vbo = mVBOs.find(type);
        NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
        auto& vertexBuffer = vbo->second;
        vertexBuffer.bufferSize = size;

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID));
        CHECK_GL(glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), (const void *)0, GL_STATIC_DRAW));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindVertexArray(0));
    }

    void Mesh::removeVertexBuffer(VertexType type) {
        const auto& vbo = mVBOs.find(type);

        if (vbo != mVBOs.end()) {
            CHECK_GL(glBindVertexArray(mVAOID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vbo->second.vboID));
            CHECK_GL(glDeleteBuffers(1, (GLuint *)&vbo->second.vboID));
            CHECK_GL(glBindVertexArray(0));
        }
        mVBOs.erase(type);
    }

    const VertexBuffer& Mesh::getVBO(VertexType type) const {
        auto vbo = mVBOs.find(type);
        NEO_ASSERT(vbo != mVBOs.end(), "Attempting to retrieve a VertexBuffer that doesn't exist");
        return vbo->second;
    }

    void Mesh::addElementBuffer(const std::vector<uint32_t>& buffer) {
        NEO_ASSERT(!mElementVBO.has_value(), "Attempting to add 2 ElementBuffers");

        mElementVBO = std::make_optional<VertexBuffer>();
        mElementVBO->bufferSize = static_cast<uint32_t>(buffer.size());

        CHECK_GL(glBindVertexArray(mVAOID));

        CHECK_GL(glGenBuffers(1, (GLuint *)&mElementVBO->vboID));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID));
        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(uint32_t), &buffer[0], GL_STATIC_DRAW));
        }
        CHECK_GL(glBindVertexArray(0));

    }

    void Mesh::updateElementBuffer(const std::vector<uint32_t>& buffer) {
        MICROPROFILE_SCOPEI("Mesh", "updateElementBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateEBO", MP_AUTO);

        NEO_ASSERT(mElementVBO.has_value() && buffer.size(), "Attempting to update an ElementBuffer that doesn't exist");
        mElementVBO->bufferSize = static_cast<uint32_t>(buffer.size());

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID));
        if (buffer.size()) {
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(uint32_t), &buffer[0], GL_STATIC_DRAW));
        }
        CHECK_GL(glBindVertexArray(0));
    }

    void Mesh::updateElementBuffer(uint32_t size) {
        MICROPROFILE_SCOPEI("Mesh", "updateElementBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateEBO", MP_AUTO);

        NEO_ASSERT(mElementVBO.has_value(), "Attempting to update an ElementBuffer that doesn't exist");
        NEO_ASSERT(size, "Attempting to update an ElementBuffer with no data");
        mElementVBO->bufferSize = size;

        CHECK_GL(glBindVertexArray(mVAOID));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID));
        CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32_t), (const void *)0, GL_STATIC_DRAW));
        CHECK_GL(glBindVertexArray(0));
    }

    void Mesh::removeElementBuffer() {
        if (mElementVBO.has_value()) {
            CHECK_GL(glBindVertexArray(mVAOID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID));
            CHECK_GL(glDeleteBuffers(1, (GLuint *)&mElementVBO->vboID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            CHECK_GL(glBindVertexArray(0));
            mElementVBO = std::nullopt;
        }

        mPrimitiveType = GL_TRIANGLE_STRIP;
    }

    void Mesh::clear() {
        removeVertexBuffer(VertexType::Position);
        removeVertexBuffer(VertexType::Normal);
        removeVertexBuffer(VertexType::Texture0);
        removeVertexBuffer(VertexType::Texture1);
        removeVertexBuffer(VertexType::Texture2);
        removeVertexBuffer(VertexType::Color0);
        removeVertexBuffer(VertexType::Color1);
        removeVertexBuffer(VertexType::Color2);
        removeElementBuffer();
    }

    void Mesh::destroy() {
        NEO_ASSERT(mVAOID, "Attempting to clear Mesh an empty mesh");
        clear();
        CHECK_GL(glDeleteVertexArrays(1, (GLuint *)&mVAOID));
    }
}