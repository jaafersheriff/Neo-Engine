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

		std::pair<GLenum, GLenum> _getGLFilter(const TextureFilter& filter) {
			GLenum min, mag;

			switch (filter.mMag) {
			case types::texture::Filters::Nearest:
				mag = GL_NEAREST;
				break;
			case types::texture::Filters::Linear:
			default:
				mag = GL_LINEAR;
				break;
			}

			if (filter.mMip == types::texture::Filters::Nearest) {
				switch (filter.mMin) {
				case types::texture::Filters::Nearest:
					min = GL_NEAREST_MIPMAP_NEAREST;
					break;
				case types::texture::Filters::Linear:
				default:
					min = GL_LINEAR_MIPMAP_NEAREST;
					break;
				}
			}
			else if (filter.mMip == types::texture::Filters::Linear) {
				switch (filter.mMin) {
				case types::texture::Filters::Nearest:
					min = GL_NEAREST_MIPMAP_LINEAR;
					break;
				case types::texture::Filters::Linear:
				default:
					min = GL_LINEAR_MIPMAP_LINEAR;
					break;
				}
			}
			else {
				NEO_FAIL("Invalid texture filter");
				min = GL_LINEAR_MIPMAP_LINEAR;
			}

			return std::make_pair(min, mag);
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

	types::texture::BaseFormats TextureFormat::deriveBaseFormat(types::texture::InternalFormats format) {
		switch (format) {
		case types::texture::InternalFormats::R8_UNORM:
		case types::texture::InternalFormats::R16_UNORM:
		case types::texture::InternalFormats::R16_UI:
		case types::texture::InternalFormats::R32_UI:
		case types::texture::InternalFormats::R16_F:
		case types::texture::InternalFormats::R32_F:
			return types::texture::BaseFormats::R;
		case types::texture::InternalFormats::RG8_UNORM:
		case types::texture::InternalFormats::RG16_UNORM:
		case types::texture::InternalFormats::RG16_UI:
		case types::texture::InternalFormats::RG16_F:
		case types::texture::InternalFormats::RG32_F:
			return types::texture::BaseFormats::RG;
		case types::texture::InternalFormats::RGB8_UNORM:
		case types::texture::InternalFormats::RGB16_UNORM:
		case types::texture::InternalFormats::RGB16_UI:
		case types::texture::InternalFormats::RGB16_F:
		case types::texture::InternalFormats::RGB32_F:
			return types::texture::BaseFormats::RGB;
		case types::texture::InternalFormats::RGBA8_UNORM:
		case types::texture::InternalFormats::RGBA16_UNORM:
		case types::texture::InternalFormats::RGBA16_UI:
		case types::texture::InternalFormats::RGBA16_F:
		case types::texture::InternalFormats::RGBA32_F:
			return types::texture::BaseFormats::RGBA;
		case types::texture::InternalFormats::D16:
		case types::texture::InternalFormats::D24:
		case types::texture::InternalFormats::D32:
			return types::texture::BaseFormats::Depth;
		case types::texture::InternalFormats::D24S8:
			return types::texture::BaseFormats::DepthStencil;
		default:
			NEO_FAIL("Invalid base format");
			return types::texture::BaseFormats::RGB;
		}
	}

	Texture::Texture(TextureFormat format, uint16_t dimension, const std::optional<std::string>& debugName, const void* data) : 
		Texture(format, glm::u16vec3(dimension, dimension, 0), debugName, data) {}

	Texture::Texture(TextureFormat format, glm::u16vec2 dimension, const std::optional<std::string>& debugName, const void* data) :
		Texture(format, glm::u16vec3(dimension.x, dimension.y, 0), debugName, data) {}

	Texture::Texture(TextureFormat format, glm::u16vec3 dimension, const std::optional<std::string>& debugName, const void* data) :
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
		if (debugName.has_value() && !debugName.value().empty()) {
			glObjectLabel(GL_TEXTURE, mTextureID, -1, debugName.value().c_str());
		}


		// Apply format
		GLenum target = _getGLTarget(format.mTarget);
		std::pair<GLenum, GLenum> glFilters = _getGLFilter(mFormat.mFilter);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, glFilters.first);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, glFilters.second);
		switch (mFormat.mTarget) {
		case types::texture::Target::Texture3D:
		case types::texture::Target::TextureCube:
			glTexParameteri(target, GL_TEXTURE_WRAP_R, _getGLWrap(mFormat.mWrap.mR));
		case types::texture::Target::Texture2D:
			glTexParameteri(target, GL_TEXTURE_WRAP_T, _getGLWrap(mFormat.mWrap.mT));
		case types::texture::Target::Texture1D:
			glTexParameteri(target, GL_TEXTURE_WRAP_S, _getGLWrap(mFormat.mWrap.mS));
			break;
		default:
			NEO_FAIL("Invalid texture class");
			break;
		}
		
		// Override mips
		uint16_t maxDim = std::max(mWidth, std::max(mHeight, mDepth));
		uint16_t mips = 0;
		while (maxDim > (1u << mips)) {
			mips++;
		}
		mips = std::max(mips, static_cast<uint16_t>(1u));
		if (mips < mFormat.mMipCount) {
			NEO_LOG_W("Too many mips requested! Overwriting %d with %d", static_cast<int>(mFormat.mMipCount), static_cast<int>(mips));
			mFormat.mMipCount = mips;
		}

		// Lock in storage
		switch (mFormat.mTarget) {
		case types::texture::Target::Texture1D:
			glTexStorage1D(GL_TEXTURE_1D, mFormat.mMipCount, GLHelper::getGLInternalFormat(mFormat.mInternalFormat), mWidth);
			break;
		case types::texture::Target::Texture2D:
			glTexStorage2D(GL_TEXTURE_2D, mFormat.mMipCount, GLHelper::getGLInternalFormat(mFormat.mInternalFormat), mWidth, mHeight);
			break;
		case types::texture::Target::Texture3D:
			glTexStorage3D(GL_TEXTURE_3D, mFormat.mMipCount, GLHelper::getGLInternalFormat(mFormat.mInternalFormat), mWidth, mHeight, mDepth);
			break;
		case types::texture::Target::TextureCube:
			glTexStorage2D(GL_TEXTURE_CUBE_MAP, mFormat.mMipCount, GLHelper::getGLInternalFormat(mFormat.mInternalFormat), mWidth, mHeight);
			break;
		default:
			NEO_FAIL("Invalid texture class");
			break;
		}

		// Upload
		if (data != nullptr) {
			types::texture::BaseFormats baseFormat = TextureFormat::deriveBaseFormat(mFormat.mInternalFormat);
			switch (mFormat.mTarget) {
			case types::texture::Target::Texture1D:
				glTexSubImage1D(GL_TEXTURE_1D, 0, 0, mWidth, _getGLBaseFormat(baseFormat), GLHelper::getGLByteFormat(mFormat.mType), data);
				break;
			case types::texture::Target::Texture2D:
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, _getGLBaseFormat(baseFormat), GLHelper::getGLByteFormat(mFormat.mType), data);
				break;
			case types::texture::Target::Texture3D:
				glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, mWidth, mHeight, mDepth, _getGLBaseFormat(baseFormat), GLHelper::getGLByteFormat(mFormat.mType), data);
				break;
			case types::texture::Target::TextureCube: {
				// Danger!
				const void** _data = reinterpret_cast<const void**>(const_cast<void*>(data));
				// F, B, U, D, R, L
				for (int i = 0; i < 6; i++) {
					NEO_ASSERT(_data[i], "Trying to upload a CubeMap with invalid data");
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, mWidth, mHeight, _getGLBaseFormat(baseFormat), GLHelper::getGLByteFormat(mFormat.mType), _data[i]);
				}
				break;
			}
			default:
				NEO_FAIL("Invalid texture class");
				break;
			}

		}
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