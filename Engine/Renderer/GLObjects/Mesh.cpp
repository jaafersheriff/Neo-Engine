#include "Renderer/pch.hpp"
#include "Mesh.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/Renderer.hpp"

#include "GL/glew.h"

namespace neo {
	namespace {
		GLenum _translatePrimitive(types::mesh::Primitive primitive) {
			switch (primitive) {
			case types::mesh::Primitive::Points:
				return GL_POINTS;
			case types::mesh::Primitive::Line:
				return GL_LINE;
			case types::mesh::Primitive::LineLoop:
				return GL_LINE_LOOP;
			case types::mesh::Primitive::LineStrip:
				return GL_LINE_STRIP;
			case types::mesh::Primitive::Triangles:
				return GL_TRIANGLES;
			case types::mesh::Primitive::TriangleStrip:
				return GL_TRIANGLE_STRIP;
			case types::mesh::Primitive::TriangleFan:
				return GL_TRIANGLE_FAN;
			default:
				NEO_FAIL("Unknown primitive type");
				return GL_TRIANGLE_FAN;
			}
		}
	}

	Mesh::Mesh(types::mesh::Primitive primitive)
		: mVAOID(0)
		, mPrimitiveType(primitive)
		, mVBOs({})
	{
	}

	// TODO - instanced
	void Mesh::draw(uint32_t size, uint16_t offset) const {

		ServiceLocator<Renderer>::value().mStats.mNumDraws++;

		glBindVertexArray(mVAOID);

		const auto& positions = getVBO(types::mesh::VertexType::Position);
		if (mElementVBO) {
			uint32_t usedSize = size ? size : mElementVBO->elementCount;
			ServiceLocator<Renderer>::value().mStats.mNumPrimitives += usedSize / positions.components;
			glDrawElements(_translatePrimitive(mPrimitiveType), usedSize, mElementVBO->format, reinterpret_cast<void*>(offset));
		}
		else if (size) {
			ServiceLocator<Renderer>::value().mStats.mNumPrimitives += size / positions.components;
			glDrawArrays(_translatePrimitive(mPrimitiveType), 0, size / positions.components);
		}
		else {
			ServiceLocator<Renderer>::value().mStats.mNumPrimitives += positions.elementCount / positions.components;
			glDrawArrays(_translatePrimitive(mPrimitiveType), 0, positions.elementCount / positions.components);
		}
	}

	void Mesh::addVertexBuffer(types::mesh::VertexType type, uint32_t components, uint32_t stride, types::ByteFormats format, bool normalized, uint32_t count, uint32_t offset, uint32_t byteSize, const uint8_t* buffer) {
		NEO_ASSERT(mVBOs.find(type) == mVBOs.end(), "Attempting to add a VertexBuffer that already exists");

		auto vertexBuffer = VertexBuffer{};
		vertexBuffer.attribArray = static_cast<uint32_t>(type);
		vertexBuffer.stride = stride;
		vertexBuffer.components = components;
		vertexBuffer.elementCount = count;
		vertexBuffer.format = GLHelper::getGLByteFormat(format);

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
		glVertexAttribPointer(vertexBuffer.attribArray, components, vertexBuffer.format, normalized ? GL_TRUE : GL_FALSE, stride, reinterpret_cast<uint8_t*>(NULL + offset));
#pragma warning(pop)

		mVBOs[type] = vertexBuffer;
	}

	void Mesh::updateVertexBuffer(types::mesh::VertexType type, uint32_t count, uint32_t byteSize, const uint8_t* data) {
		const auto& vbo = mVBOs.find(type);
		NEO_ASSERT(vbo != mVBOs.end(), "Attempting to update a VertexBuffer that doesn't exist");
		auto& vertexBuffer = vbo->second;
		vertexBuffer.elementCount = count;

		glBindVertexArray(mVAOID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboID);
		if (byteSize) {
			glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_DYNAMIC_DRAW);
		}
	}

	void Mesh::removeVertexBuffer(types::mesh::VertexType type) {
		const auto& vbo = mVBOs.find(type);

		if (vbo != mVBOs.end()) {
			glBindVertexArray(mVAOID);
			glBindBuffer(GL_ARRAY_BUFFER, vbo->second.vboID);
			glDeleteBuffers(1, (GLuint *)&vbo->second.vboID);
		}
		mVBOs.erase(type);
	}

	bool Mesh::hasVBO(types::mesh::VertexType type) const {
		return mVBOs.find(type) != mVBOs.end();
	}

	const VertexBuffer& Mesh::getVBO(types::mesh::VertexType type) const {
		auto vbo = mVBOs.find(type);
		NEO_ASSERT(vbo != mVBOs.end(), "Attempting to retrieve a VertexBuffer that doesn't exist");
		return vbo->second;
	}

	void Mesh::addElementBuffer(uint32_t count, types::ByteFormats format, uint32_t byteSize, const uint8_t* data) {
		NEO_ASSERT(!mElementVBO.has_value(), "Attempting to add 2 ElementBuffers");

		mElementVBO = std::make_optional<VertexBuffer>();
		mElementVBO->elementCount = count;
		mElementVBO->stride = 0;
		mElementVBO->components = 1;
		mElementVBO->format = GLHelper::getGLByteFormat(format);

		glBindVertexArray(mVAOID);

		glGenBuffers(1, (GLuint *)&mElementVBO->vboID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
		if (byteSize) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
		}
	}

	void Mesh::removeElementBuffer() {
		if (mElementVBO.has_value()) {
			glBindVertexArray(mVAOID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementVBO->vboID);
			glDeleteBuffers(1, (GLuint *)&mElementVBO->vboID);
			mElementVBO = std::nullopt;
		}

		if (mPrimitiveType == types::mesh::Primitive::Triangles) {
			mPrimitiveType = types::mesh::Primitive::TriangleStrip;
		}
	}

	void Mesh::clear() {
		removeVertexBuffer(types::mesh::VertexType::Position);
		removeVertexBuffer(types::mesh::VertexType::Normal);
		removeVertexBuffer(types::mesh::VertexType::Texture0);
		removeVertexBuffer(types::mesh::VertexType::Tangent);
		removeElementBuffer();
	}

	void Mesh::init(const std::optional<std::string>& debugName) {
		glGenVertexArrays(1, (GLuint*)&mVAOID);
		if (debugName.has_value() && !debugName.value().empty()) {
			glBindVertexArray(mVAOID);
			glObjectLabel(GL_VERTEX_ARRAY, mVAOID, -1, debugName.value().c_str());
		}
	}

	void Mesh::destroy() {
		NEO_ASSERT(mVAOID, "Attempting to clear Mesh an empty mesh");
		clear();
		glDeleteVertexArrays(1, (GLuint *)&mVAOID);
	}
}
