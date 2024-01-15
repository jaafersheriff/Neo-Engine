#include "Renderer/pch.hpp"
#include "Texture.hpp"

namespace neo {
	namespace {
		GLenum _getGLTarget(TextureTarget target) {
			switch (target) {
			case TextureTarget::Texture1D:
				return GL_TEXTURE_1D;
			case TextureTarget::Texture2D:
				return GL_TEXTURE_2D;
			case TextureTarget::Texture3D:
				return GL_TEXTURE_3D;
			case TextureTarget::TextureCube:
				return GL_TEXTURE_CUBE_MAP;
			default:
				NEO_FAIL("Invalid texture class");
				return 0;
			}
		}
	}

	Texture::Texture(TextureFormat format, uint16_t dimension, const void* data) : 
		Texture(format, glm::u16vec3(dimension, dimension, 0), data) {}

	Texture::Texture(TextureFormat format, glm::u16vec2 dimension, const void* data) :
		Texture(format, glm::u16vec3(dimension.x, dimension.y, 0), data) {}

	Texture::Texture(TextureFormat format, glm::u16vec3 dimension, const void* data) :
		mFormat(format) {
		switch (mFormat.mTarget) {
		case TextureTarget::Texture3D:
			mDepth = dimension.z;
		case TextureTarget::TextureCube:
		case TextureTarget::Texture2D:
			mHeight = dimension.y;
		case TextureTarget::Texture1D:
			mWidth = dimension.x;
			break;
		default:
			NEO_FAIL("Invalid texture class");
			break;
		}

		glGenTextures(1, &mTextureID);
		bind();

		// Apply format
		GLenum target = _getGLTarget(format.mTarget);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mFormat.mFilter);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, mFormat.mFilter);
		switch (mFormat.mTarget) {
		case TextureTarget::Texture3D:
            glTexParameteri(target, GL_TEXTURE_WRAP_R, mFormat.mMode);
		case TextureTarget::TextureCube:
		case TextureTarget::Texture2D:
            glTexParameteri(target, GL_TEXTURE_WRAP_T, mFormat.mMode);
		case TextureTarget::Texture1D:
            glTexParameteri(target, GL_TEXTURE_WRAP_S, mFormat.mMode);
			break;
		default:
			NEO_FAIL("Invalid texture class");
			break;
		}

		// Lock in storage
		switch (mFormat.mTarget) {
		case TextureTarget::Texture1D:
			glTexStorage1D(GL_TEXTURE_1D, 1, mFormat.mInternalFormat, mWidth);
			break;
		case TextureTarget::Texture2D:
			glTexStorage2D(GL_TEXTURE_2D, 1, mFormat.mInternalFormat, mWidth, mHeight);
			break;
		case TextureTarget::Texture3D:
			glTexStorage3D(GL_TEXTURE_3D, 1, mFormat.mInternalFormat, mWidth, mHeight, mDepth);
			break;
		case TextureTarget::TextureCube:
			for (int i = 0; i < 6; i++) {
				glTexStorage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 1, mFormat.mInternalFormat, mWidth, mHeight);
			}
			break;
		default:
			NEO_FAIL("Invalid texture class");
			break;
		}

		// Upload
		if (data != nullptr) {
			switch (mFormat.mTarget) {
			case TextureTarget::Texture1D:
				glTexSubImage1D(GL_TEXTURE_1D, 0, 0, mWidth, mFormat.mBaseFormat, mFormat.mType, data);
				break;
			case TextureTarget::Texture2D:
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, mFormat.mBaseFormat, mFormat.mType, data);
				break;
			case TextureTarget::Texture3D:
				glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, mWidth, mHeight, mDepth, mFormat.mBaseFormat, mFormat.mType, data);
				break;
			case TextureTarget::TextureCube: {
				// Danger!
				const void** _data = reinterpret_cast<const void**>(const_cast<void*>(data));
				// F, B, U, D, R, L
				for (int i = 0; i < 6; i++) {
					NEO_ASSERT(_data[i], "Trying to upload a CubeMap with invalid data");
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, mWidth, mHeight, mFormat.mBaseFormat, mFormat.mType, _data[i]);
				}
				break;
			}
			default:
				NEO_FAIL("Invalid texture class");
				break;
			}

		}

        NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when creating Texture");
	}

	void Texture::bind() const {
		glBindTexture(_getGLTarget(mFormat.mTarget), mTextureID);
	}

	void Texture::genMips() {
		glGenerateTextureMipmap(mTextureID);
	}

	void Texture::destroy() {
		glDeleteTextures(1, &mTextureID);
		mTextureID = 0;
		mWidth = 1;
		mHeight = 1;
		mDepth = 0;
	}

}