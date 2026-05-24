#include "Renderer/pch.hpp"
#include "ShaderBuffer.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"

#include "GL/glew.h"

namespace neo {

	ShaderBuffer::ShaderBuffer(uint32_t byteSize, const uint8_t* data, const std::optional<std::string>& debugName) 
		: mByteSize(byteSize)
	{
		glGenBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
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
		GLenum baseFormat;
		if (format == types::ByteFormats::Float) {
			baseFormat = GL_RED;
		}
		else {
			baseFormat = GL_RED_INTEGER;
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
