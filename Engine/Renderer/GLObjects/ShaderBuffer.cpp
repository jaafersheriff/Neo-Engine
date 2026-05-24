#include "Renderer/pch.hpp"
#include "ShaderBuffer.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"

#include "GL/glew.h"

namespace neo {
	namespace {
		GLenum _translateTarget(types::buffer::Target target) {
			switch (target) {
			case types::buffer::Target::Uniform:
				return GL_UNIFORM_BUFFER;
			case types::buffer::Target::ShaderStorage:
				return GL_SHADER_STORAGE_BUFFER;
			default:
				NEO_FAIL("Unknown target type");
				return GL_SHADER_STORAGE_BUFFER;
			}
		}
	}

	ShaderBuffer::ShaderBuffer(types::buffer::Target target, uint32_t byteSize, const uint8_t* data, const std::optional<std::string>& debugName) 
		: mTarget(target)
		, mByteSize(byteSize)
	{
		glGenBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
		GLenum glTarget = _translateTarget(target);
		glBindBuffer(glTarget, mBufferID);

		if (debugName.has_value() && !debugName.value().empty()) {
			glObjectLabel(GL_BUFFER, mBufferID, -1, debugName.value().c_str());
		}
		glBufferData(glTarget, byteSize, data, GL_DYNAMIC_DRAW);
		glBindBuffer(glTarget, 0);
	}

	void ShaderBuffer::update(uint32_t byteSize, const uint8_t* data, uint32_t offset) {
		NEO_ASSERT(offset + byteSize <= mByteSize, "Shader buffer update out of bounds");
		GLenum glTarget = _translateTarget(mTarget);
		glBindBuffer(glTarget, mBufferID);
		if (byteSize) {
			TRACY_GPUN("glBufferSubData");
			glBufferSubData(glTarget, offset, byteSize, data);
		}
		glBindBuffer(glTarget, 0);
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
		GLenum target = _translateTarget(mTarget);;
		glBindBuffer(target, mBufferID);
		glClearBufferSubData(
			target,
			GLHelper::getGLInternalFormat(internalFormat),
			offset,
			byteSize,
			baseFormat,
			GLHelper::getGLByteFormat(format),
			clearValue
		);
		glBindBuffer(target, 0);
	}

	void ShaderBuffer::destroy() {
		if (mBufferID) {
			glDeleteBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
			mBufferID = 0;
			mByteSize = 0;
		}
	}
}
