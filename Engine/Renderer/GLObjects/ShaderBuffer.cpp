#include "Renderer/pch.hpp"
#include "ShaderBuffer.hpp"

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
	{
		glGenBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
		if (debugName.has_value() && !debugName.value().empty()) {
			glObjectLabel(GL_BUFFER, mBufferID, -1, debugName.value().c_str());
		}

		mByteSize = byteSize;
		GLenum glTarget = _translateTarget(target);
		glBindBuffer(glTarget, mBufferID);
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

	void ShaderBuffer::destroy() {
		if (mBufferID) {
			glDeleteBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
			mBufferID = 0;
			mByteSize = 0;
		}
	}
}
