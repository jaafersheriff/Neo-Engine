#include "Renderer/pch.hpp"
#include "Texture.hpp"

#include "Renderer/GLObjects/GLHelper.hpp"

#include "GL/glew.h"

namespace neo {
	namespace {
		GLenum _getGLTarget(types::texture::Target target) {
			switch (target) {
			case types::texture::Target::Texture1D:
				return GL_TEXTURE_1D;
			case types::texture::Target::Texture2D:
				return GL_TEXTURE_2D;
			case types::texture::Target::Texture3D:
				return GL_TEXTURE_3D;
			case types::texture::Target::TextureCube:
				return GL_TEXTURE_CUBE_MAP;
			default:
				NEO_FAIL("Invalid texture type");
				return 0;
			}
		}

		GLenum _getGLFilter(types::texture::Filters filter) {
			switch (filter) {
			case types::texture::Filters::Linear:
				return GL_LINEAR;
			case types::texture::Filters::Nearest:
				return GL_NEAREST;
			case types::texture::Filters::NearestMipmapNearest:
				return GL_NEAREST_MIPMAP_NEAREST;
			case types::texture::Filters::LinearMipmapNearest:
				return GL_LINEAR_MIPMAP_NEAREST;
			case types::texture::Filters::NearestMipmapLinear:
				return GL_NEAREST_MIPMAP_LINEAR;
			case types::texture::Filters::LinearMipmapLinear:
				return GL_LINEAR_MIPMAP_LINEAR;
			default:
				NEO_FAIL("Invalid texture filter");
				return 0;
			}
		}

		GLenum _getGLWrap(types::texture::Wraps wrap) {
			switch (wrap) {
			case types::texture::Wraps::Clamp:
				return GL_CLAMP_TO_EDGE;
			case types::texture::Wraps::Mirrored:
				return GL_MIRRORED_REPEAT;
			case types::texture::Wraps::Repeat:
				return GL_REPEAT;
			default:
				NEO_FAIL("Invalid texture wrap");
				return 0;
			}
		}

		GLenum _getGLInternalFormat(types::texture::InternalFormats format) {
			switch (format) {
			case types::texture::InternalFormats::R8_UNORM: return GL_R8;
			case types::texture::InternalFormats::RG8_UNORM: return GL_RG8;
			case types::texture::InternalFormats::RGB8_UNORM: return GL_RGB8;
			case types::texture::InternalFormats::RGBA8_UNORM: return GL_RGBA8;
			case types::texture::InternalFormats::R16_UNORM: return GL_R16;
			case types::texture::InternalFormats::RG16_UNORM: return GL_RG16;
			case types::texture::InternalFormats::RGB16_UNORM: return GL_RGB16;
			case types::texture::InternalFormats::RGBA16_UNORM: return GL_RGBA16;
			case types::texture::InternalFormats::R16_UI: return GL_R16UI;
			case types::texture::InternalFormats::RG16_UI: return GL_RG16UI;
			case types::texture::InternalFormats::RGB16_UI: return GL_RGB16UI;
			case types::texture::InternalFormats::RGBA16_UI: return GL_RGBA16UI;
			case types::texture::InternalFormats::R16_F: return GL_R16F;
			case types::texture::InternalFormats::RG16_F: return GL_RG16F;
			case types::texture::InternalFormats::RGB16_F: return GL_RGB16F;
			case types::texture::InternalFormats::RGBA16_F: return GL_RGBA16F;
			case types::texture::InternalFormats::R32_F: return GL_R32F;
			case types::texture::InternalFormats::RG32_F: return GL_RG32F;
			case types::texture::InternalFormats::RGB32_F: return GL_RGB32F;
			case types::texture::InternalFormats::RGBA32_F: return GL_RGBA32F;
			case types::texture::InternalFormats::D16: return GL_DEPTH_COMPONENT16;
			case types::texture::InternalFormats::D24: return GL_DEPTH_COMPONENT24;
			case types::texture::InternalFormats::D32: return GL_DEPTH_COMPONENT32F;
			case types::texture::InternalFormats::D24S8: return GL_DEPTH24_STENCIL8;
			default:
				NEO_FAIL("Invalid internal format");
				return GL_RGB8;
			}
		}

		GLenum _getGLBaseFormat(types::texture::BaseFormats format) {
			switch (format) {
			case types::texture::BaseFormats::R: return GL_RED;
			case types::texture::BaseFormats::RG: return GL_RG;
			case types::texture::BaseFormats::RGB: return GL_RGB;
			case types::texture::BaseFormats::RGBA: return GL_RGBA;
			case types::texture::BaseFormats::Depth: return GL_DEPTH_COMPONENT;
			case types::texture::BaseFormats::DepthStencil: return GL_DEPTH_STENCIL;
			default:
				NEO_FAIL("Invalid base format");
				return GL_RGB;
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
		case types::texture::Target::Texture3D:
			mDepth = dimension.z;
		case types::texture::Target::TextureCube:
		case types::texture::Target::Texture2D:
			mHeight = dimension.y;
		case types::texture::Target::Texture1D:
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
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, _getGLFilter(mFormat.mFilter.mMin));
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, _getGLFilter(mFormat.mFilter.mMag));
		switch (mFormat.mTarget) {
		case types::texture::Target::Texture3D:
			glTexParameteri(target, GL_TEXTURE_WRAP_R, _getGLWrap(mFormat.mWrap.mR));
		case types::texture::Target::TextureCube:
		case types::texture::Target::Texture2D:
			glTexParameteri(target, GL_TEXTURE_WRAP_T, _getGLWrap(mFormat.mWrap.mT));
		case types::texture::Target::Texture1D:
			glTexParameteri(target, GL_TEXTURE_WRAP_S, _getGLWrap(mFormat.mWrap.mS));
			break;
		default:
			NEO_FAIL("Invalid texture class");
			break;
		}

		// Lock in storage
		switch (mFormat.mTarget) {
		case types::texture::Target::Texture1D:
			glTexStorage1D(GL_TEXTURE_1D, 1, _getGLInternalFormat(mFormat.mInternalFormat), mWidth);
			break;
		case types::texture::Target::Texture2D:
			glTexStorage2D(GL_TEXTURE_2D, 1, _getGLInternalFormat(mFormat.mInternalFormat), mWidth, mHeight);
			break;
		case types::texture::Target::Texture3D:
			glTexStorage3D(GL_TEXTURE_3D, 1, _getGLInternalFormat(mFormat.mInternalFormat), mWidth, mHeight, mDepth);
			break;
		case types::texture::Target::TextureCube:
			for (int i = 0; i < 6; i++) {
				glTexStorage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 1, _getGLInternalFormat(mFormat.mInternalFormat), mWidth, mHeight);
			}
			break;
		default:
			NEO_FAIL("Invalid texture class");
			break;
		}

		// Upload
		if (data != nullptr) {
			switch (mFormat.mTarget) {
			case types::texture::Target::Texture1D:
				glTexSubImage1D(GL_TEXTURE_1D, 0, 0, mWidth, _getGLBaseFormat(mFormat.mBaseFormat), GLHelper::getGLByteFormat(mFormat.mType), data);
				break;
			case types::texture::Target::Texture2D:
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, _getGLBaseFormat(mFormat.mBaseFormat), GLHelper::getGLByteFormat(mFormat.mType), data);
				break;
			case types::texture::Target::Texture3D:
				glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, mWidth, mHeight, mDepth, _getGLBaseFormat(mFormat.mBaseFormat), GLHelper::getGLByteFormat(mFormat.mType), data);
				break;
			case types::texture::Target::TextureCube: {
				// Danger!
				const void** _data = reinterpret_cast<const void**>(const_cast<void*>(data));
				// F, B, U, D, R, L
				for (int i = 0; i < 6; i++) {
					NEO_ASSERT(_data[i], "Trying to upload a CubeMap with invalid data");
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, mWidth, mHeight, _getGLBaseFormat(mFormat.mBaseFormat), GLHelper::getGLByteFormat(mFormat.mType), _data[i]);
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