#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace neo {

	enum class TextureTarget {
		Texture1D,
		Texture2D,
		Texture3D,
		TextureCube
	};

	struct TextureFormat {
		TextureTarget mTarget = TextureTarget::Texture2D;
        GLint mInternalFormat = GL_RGBA8;
        GLenum mBaseFormat = GL_RGBA;
        GLint mFilter = GL_LINEAR; // TODO - this should be split between min/mag/mip
        GLenum mMode = GL_REPEAT; // TODO - this should be split between STR
        GLenum mType = GL_UNSIGNED_BYTE;

		bool operator==(const TextureFormat& other) const noexcept {
			return mTarget == other.mTarget
				&& mInternalFormat == other.mInternalFormat
				&& mBaseFormat == other.mBaseFormat
				&& mFilter == other.mFilter
				&& mMode == other.mMode
				&& mType == other.mType
			;
		}
	};

	class Texture {
	public:
		Texture(TextureFormat format, uint16_t dimension, const void* data = nullptr);
		Texture(TextureFormat format, glm::u16vec2 dimension, const void* data = nullptr);
		Texture(TextureFormat format, glm::u16vec3 dimension, const void* data = nullptr);

		void bind() const;
		void genMips();
		void destroy();

		GLuint mTextureID = 0;
		const TextureFormat mFormat; // TODO - can this be const?

		uint16_t mWidth = 1;
		uint16_t mHeight = 1;
		uint16_t mDepth = 0;
	};
}