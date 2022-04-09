#include "Mesh.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/Renderer.hpp"

#include "Util/Util.hpp"
#include "Util/ServiceLocator.hpp"

#include "GL/glew.h"
#include "microprofile.h"

namespace neo {

    Mesh::Mesh(int primitiveType)
        : mVAOID(0)
        , mPrimitiveType(primitiveType < 0 ? GL_TRIANGLE_STRIP : primitiveType)
        , mVBOs({})
    {
        /* Initialize VAO */
        glGenVertexArrays(1, (GLuint *)&mVAOID);
    }

    Mesh::~Mesh() {
        destroy();
    }

    // TODO - instanced
    void Mesh::draw(uint32_t instances, uint32_t count) const {
        ServiceLocator<Renderer>::ref().mStats.mNumDraws++;

        const auto& positions = getVBO(VertexType::Position);

        glBindVertexArray(mVAOID);
        if (mElementVBO) {
            uint32_t usedSize = count ? count : mElementVBO->bufferSize;
            ServiceLocator<Renderer>::ref().mStats.mNumTriangles += instances * usedSize / 3;
            glDrawElementsInstanced(mPrimitiveType, usedSize, GL_UNSIGNED_INT, nullptr, instances);
        }
        else if (count) {
            ServiceLocator<Renderer>::ref().mStats.mNumTriangles += count / 3;
            glDrawArrays(mPrimitiveType, 0, count);
        }
        else {
            ServiceLocator<Renderer>::ref().mStats.mNumTriangles += positions.bufferSize / positions.size / 3;
            glDrawArrays(mPrimitiveType, 0, positions.bufferSize / positions.size);
        }
    }

    void Mesh::addVertexBuffer(VertexType type, uint32_t attribArray, uint32_t size, const std::vector<float>& buffer) {
        NEO_ASSERT(mVBOs.find(type) == mVBOs.end(), "Attempting to add a VertexBuffer that already exists");

        auto vertexBuffer = VertexBuffer{};
        vertexBuffer.attribArray = attribArray;
        vertexBuffer.size = size;
        vertexBuffer.bufferSize = static_cast<uint32_t>(buffer.size());

        glBindVertexArray(mVAOID);
        glGenBuffers(1, (GLuint *)&vertexBuffer.vboID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);

        if (buffer.size()) {
            glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_STATIC_DRAW);
        }
        glEnableVertexAttribArray(attribArray);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
        glVertexAttribPointer(attribArray, size, GL_FLOAT, GL_FALSE, 0, (const void *)0);

        mVBOs[type] = vertexBuffer;
    }

    void Mesh::updateVertexBuffer(VertexType type, void* buffer, size_t size) {
        MICROPROFILE_SCOPEI("Mesh", "updateVertexBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateVertexBuffer", MP_AUTO);

        const auto& vbo = mVBOs.find(type);
        NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
        auto& vertexBuffer = vbo->second;
        vertexBuffer.bufferSize = static_cast<uint32_t>(size);

        glBindVertexArray(mVAOID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
        if (size) {
            // TODO _ this is bad
            glBufferData(GL_ARRAY_BUFFER, size * vertexBuffer.size * sizeof(float), buffer, GL_STATIC_DRAW);
        }
    }

    void Mesh::updateVertexBuffer(VertexType type, const std::vector<float>& buffer) {
        MICROPROFILE_SCOPEI("Mesh", "updateVertexBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateVertexBuffer", MP_AUTO);

        const auto& vbo = mVBOs.find(type);
        NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
        auto& vertexBuffer = vbo->second;
        vertexBuffer.bufferSize = static_cast<uint32_t>(buffer.size());

        glBindVertexArray(mVAOID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
        if (buffer.size()) {
            glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_STATIC_DRAW);
        }
    }

    void Mesh::updateVertexBuffer(VertexType type, uint32_t size) {
        MICROPROFILE_SCOPEI("Mesh", "updateVertexBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateVBO", MP_AUTO);

        NEO_ASSERT(size > 0, "Attempting to update a VertexBuffer with no data");
        const auto& vbo = mVBOs.find(type);
        NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
        auto& vertexBuffer = vbo->second;
        vertexBuffer.bufferSize = size;

        glBindVertexArray(mVAOID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), (const void *)0, GL_STATIC_DRAW);
    }

    void Mesh::removeVertexBuffer(VertexType type) {
        const auto& vbo = mVBOs.find(type);

        if (vbo != mVBOs.end()) {
            glBindVertexArray(mVAOID);
            glBindBuffer(GL_ARRAY_BUFFER, vbo->second.vboID);
            glDeleteBuffers(1, (GLuint *)&vbo->second.vboID);
            glBindVertexArray(0);
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

        glBindVertexArray(mVAOID);

        glGenBuffers(1, (GLuint *)&mElementVBO->vboID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
        if (buffer.size()) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(uint32_t), &buffer[0], GL_STATIC_DRAW);
        }

    }

    void Mesh::updateElementBuffer(const std::vector<uint32_t>& buffer) {
        MICROPROFILE_SCOPEI("Mesh", "updateElementBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateEBO", MP_AUTO);

        NEO_ASSERT(mElementVBO.has_value() && buffer.size(), "Attempting to update an ElementBuffer that doesn't exist");
        mElementVBO->bufferSize = static_cast<uint32_t>(buffer.size());

        glBindVertexArray(mVAOID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
        if (buffer.size()) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(uint32_t), &buffer[0], GL_STATIC_DRAW);
        }
    }

    void Mesh::updateElementBuffer(uint32_t size) {
        MICROPROFILE_SCOPEI("Mesh", "updateElementBuffer", MP_AUTO);
        MICROPROFILE_SCOPEGPUI("Mesh::updateEBO", MP_AUTO);

        NEO_ASSERT(mElementVBO.has_value(), "Attempting to update an ElementBuffer that doesn't exist");
        NEO_ASSERT(size, "Attempting to update an ElementBuffer with no data");
        mElementVBO->bufferSize = size;

        glBindVertexArray(mVAOID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32_t), (const void *)0, GL_STATIC_DRAW);
    }

    void Mesh::removeElementBuffer() {
        if (mElementVBO.has_value()) {
            glBindVertexArray(mVAOID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
            glDeleteBuffers(1, (GLuint *)&mElementVBO->vboID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
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
        glDeleteVertexArrays(1, (GLuint *)&mVAOID);
        glBindVertexArray(0);
    }
}