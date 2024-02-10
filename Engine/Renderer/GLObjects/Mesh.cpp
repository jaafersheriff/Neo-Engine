#include "Renderer/pch.hpp"
#include "Mesh.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/Renderer.hpp"

#include "GL/glew.h"

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
	void Mesh::draw(uint32_t size) const {

		ServiceLocator<Renderer>::ref().mStats.mNumDraws++;


		glBindVertexArray(mVAOID);

		if (mElementVBO) {
			uint32_t usedSize = size ? size : mElementVBO->elementCount;
			ServiceLocator<Renderer>::ref().mStats.mNumTriangles += usedSize / 3;
			glDrawElements(mPrimitiveType, usedSize, mElementVBO->format, nullptr);
		}
		else if (size) {
			ServiceLocator<Renderer>::ref().mStats.mNumTriangles += size / 3;
			glDrawArrays(mPrimitiveType, 0, size);
		}
		else {
			const auto& positions = getVBO(VertexType::Position);
			ServiceLocator<Renderer>::ref().mStats.mNumTriangles += positions.elementCount / positions.stride / 3;
			glDrawArrays(mPrimitiveType, 0, positions.elementCount);
		}
	}

	void Mesh::addVertexBuffer_DEPRECATED(VertexType type, uint32_t attribArray, uint32_t stride, const std::vector<float>& buffer) {
		NEO_ASSERT(mVBOs.find(type) == mVBOs.end(), "Attempting to add a VertexBuffer that already exists");

		auto vertexBuffer = VertexBuffer{};
		vertexBuffer.attribArray = attribArray;
		vertexBuffer.stride = stride;
		vertexBuffer.elementCount = static_cast<uint32_t>(buffer.size());
		vertexBuffer.format = GL_FLOAT;

		glBindVertexArray(mVAOID);
		glGenBuffers(1, (GLuint *)&vertexBuffer.vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);

		if (buffer.size()) {
			glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_STATIC_DRAW);
		}
		glEnableVertexAttribArray(attribArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
		glVertexAttribPointer(attribArray, stride, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		mVBOs[type] = vertexBuffer;
	}

	void Mesh::addVertexBuffer(VertexType type, uint32_t components, uint32_t stride, uint32_t format, bool normalized, uint32_t count, uint32_t offset, uint32_t byteSize, const uint8_t* buffer) {
		NEO_ASSERT(mVBOs.find(type) == mVBOs.end(), "Attempting to add a VertexBuffer that already exists");

		auto vertexBuffer = VertexBuffer{};
		vertexBuffer.attribArray = static_cast<uint32_t>(type);
		vertexBuffer.stride = stride;
		vertexBuffer.elementCount = count;
		vertexBuffer.format = format;

		glBindVertexArray(mVAOID);
		glGenBuffers(1, (GLuint*)&vertexBuffer.vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);

		if (byteSize) {
			glBufferData(GL_ARRAY_BUFFER, byteSize, buffer, GL_STATIC_DRAW);
		}
		glEnableVertexAttribArray(vertexBuffer.attribArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);

#pragma warning(push)
#pragma warning(disable: 4312)
		glVertexAttribPointer(vertexBuffer.attribArray, components, format, normalized ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<uint8_t*>(NULL + offset));
#pragma warning(pop)

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		mVBOs[type] = vertexBuffer;
	}

	void Mesh::updateVertexBuffer_DEPRECATED(VertexType type, const std::vector<float>& buffer) {
		TRACY_GPU();

		const auto& vbo = mVBOs.find(type);
		NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
		auto& vertexBuffer = vbo->second;
		vertexBuffer.elementCount = static_cast<uint32_t>(buffer.size());

		glBindVertexArray(mVAOID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
		if (buffer.size()) {
			glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_DYNAMIC_DRAW);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Mesh::updateVertexBuffer_DEPRECATED(VertexType type, uint32_t size) {
		TRACY_GPU();

		NEO_ASSERT(size > 0, "Attempting to update a VertexBuffer with no data");
		const auto& vbo = mVBOs.find(type);
		NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
		auto& vertexBuffer = vbo->second;
		vertexBuffer.elementCount = size;

		glBindVertexArray(mVAOID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), (const void *)0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
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

	void Mesh::addElementBuffer_DEPRECATED(const std::vector<uint32_t>& buffer) {
		NEO_ASSERT(!mElementVBO.has_value(), "Attempting to add 2 ElementBuffers");

		mElementVBO = std::make_optional<VertexBuffer>();
		mElementVBO->elementCount = static_cast<uint32_t>(buffer.size());
		mElementVBO->format = GL_UNSIGNED_INT;

		glBindVertexArray(mVAOID);

		glGenBuffers(1, (GLuint *)&mElementVBO->vboID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
		if (buffer.size()) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(uint32_t), &buffer[0], GL_STATIC_DRAW);
		}
		glBindVertexArray(0);
	}

	void Mesh::addElementBuffer(uint32_t count, uint32_t format, uint32_t byteSize, const uint8_t* data) {
		NEO_ASSERT(!mElementVBO.has_value(), "Attempting to add 2 ElementBuffers");

		mElementVBO = std::make_optional<VertexBuffer>();
		mElementVBO->stride = 1;
		mElementVBO->elementCount = count;
		mElementVBO->format = format;

		glBindVertexArray(mVAOID);

		glGenBuffers(1, (GLuint *)&mElementVBO->vboID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
		if (byteSize) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
		}
		glBindVertexArray(0);
	}

	void Mesh::updateElementBuffer_DEPRECATED(const std::vector<uint32_t>& buffer) {
		TRACY_GPU();

		NEO_ASSERT(mElementVBO.has_value() && buffer.size(), "Attempting to update an ElementBuffer that doesn't exist");
		mElementVBO->elementCount = static_cast<uint32_t>(buffer.size());

		glBindVertexArray(mVAOID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
		if (buffer.size()) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(uint32_t), &buffer[0], GL_DYNAMIC_DRAW);
		}
		glBindVertexArray(0);
	}

	void Mesh::updateElementBuffer_DEPRECATED(uint32_t size) {
		TRACY_GPU();

		NEO_ASSERT(mElementVBO.has_value(), "Attempting to update an ElementBuffer that doesn't exist");
		NEO_ASSERT(size, "Attempting to update an ElementBuffer with no data");
		mElementVBO->elementCount = size;

		glBindVertexArray(mVAOID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32_t), (const void *)0, GL_DYNAMIC_DRAW);
		glBindVertexArray(0);
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
	}
}