#include "Renderer/pch.hpp"
#include "ShaderBuffer.hpp"

#include "GL/glew.h"

namespace neo {

	ShaderBuffer::ShaderBuffer(uint32_t byteSize, const uint8_t* data, const std::optional<std::string>& debugName) {
		glGenBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
		if (debugName.has_value() && !debugName.value().empty()) {
			glObjectLabel(GL_BUFFER, mBufferID, -1, debugName.value().c_str());
		}

		mByteSize = byteSize;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_DYNAMIC_DRAW);
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

	void ShaderBuffer::destroy() {
		if (mBufferID) {
			glDeleteBuffers(1, reinterpret_cast<GLuint*>(&mBufferID));
			mBufferID = 0;
			mByteSize = 0;
		}
	}
}
