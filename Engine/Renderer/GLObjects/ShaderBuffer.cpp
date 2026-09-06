#include "Renderer/pch.hpp"
#include "ShaderBuffer.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"

#include "GL/glew.h"

namespace neo {

	ShaderBuffer::ShaderBuffer(uint32_t byteSize, const uint8_t* data, const std::optional<std::string>& debugName)
		: mByteSize(byteSize)
	{
		glGenBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
		// Bound before the label: glGenBuffers only reserves a name, and the object does not exist for
		// glObjectLabel to name until it has been bound once.
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferID);
		if (debugName.has_value() && !debugName.value().empty()) {
			glObjectLabel(GL_BUFFER, mBufferID, -1, debugName.value().c_str());
		}
		glBufferData(GL_SHADER_STORAGE_BUFFER, mByteSize, data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderBuffer::update(uint32_t byteSize, const uint8_t* data, uint32_t offset) {
		NEO_ASSERT(offset + byteSize <= mByteSize, "Shader buffer update out of bounds");
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferID);
		if (byteSize) {
			TRACY_GPUN("glBufferSubData");
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderBuffer::_clear(uint32_t byteSize, uint32_t offset, types::InternalFormats internalFormat, types::ByteFormats format, const uint8_t* clearValue) {
		TRACY_GPUN("glClearBufferSubData");
		GLenum baseFormat = GL_RED;

		// Same rule as a texture upload: an integer internal format has to be paired with an integer
		// base format rather than GL_RED.
		switch (format) {
		case types::ByteFormats::Float:
			baseFormat = GL_RED;
			break;
		case types::ByteFormats::Int:
		case types::ByteFormats::UnsignedInt:
			baseFormat = GL_RED_INTEGER;
			break;
		default:
			NEO_FAIL("Unsupported byte format");
			break;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferID);
		glClearBufferSubData(
			GL_SHADER_STORAGE_BUFFER,
			GLHelper::getGLInternalFormat(internalFormat),
			offset,
			byteSize,
			baseFormat,
			GLHelper::getGLByteFormat(format),
			clearValue
		);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderBuffer::destroy() {
		if (mBufferID) {
			glDeleteBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
			mBufferID = 0;
			mByteSize = 0;
		}
	}
}
