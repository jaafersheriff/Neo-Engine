#pragma once

#include "Renderer/Types.hpp"

#include <glm/glm.hpp>

namespace neo {

	struct TextureFilter {
		types::texture::Filters mMin = types::texture::Filters::Linear;
		types::texture::Filters mMag = types::texture::Filters::Linear;
		//types::texture::Filters mMip = types::texture::Filters::Linear;

		bool operator==(const TextureFilter& other) const noexcept {
			return mMin == other.mMin
				&& mMag == other.mMag;
				//&& mMip == other.mMip;
		}
	};

	struct TextureWrap {
		types::texture::Wraps mS = types::texture::Wraps::Clamp;
		types::texture::Wraps mT = types::texture::Wraps::Clamp;
		types::texture::Wraps mR = types::texture::Wraps::Clamp;

		bool operator==(const TextureWrap& other) const noexcept {
			return mS == other.mS
				&& mT == other.mR
				&& mR == other.mT;
		}
	};

	struct TextureFormat {
		types::texture::Target mTarget = types::texture::Target::Texture2D;
		types::texture::InternalFormats mInternalFormat = types::texture::InternalFormats::RGBA8;
		types::texture::BaseFormats mBaseFormat = types::texture::BaseFormats::RGBA;
		TextureFilter mFilter = {
			types::texture::Filters::Linear,
			types::texture::Filters::Linear
			//types::texture::Filters::Linear
		};
		TextureWrap mWrap = {
			types::texture::Wraps::Clamp,
			types::texture::Wraps::Clamp,
			types::texture::Wraps::Clamp
		};
		types::ByteFormats mType = types::ByteFormats::UnsignedByte;

		bool operator==(const TextureFormat& other) const noexcept {
			return mTarget == other.mTarget
				&& mInternalFormat == other.mInternalFormat
				&& mBaseFormat == other.mBaseFormat
				&& mFilter == other.mFilter
				&& mWrap == other.mWrap
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

		uint32_t mTextureID = 0;
		const TextureFormat mFormat;

		uint16_t mWidth = 1;
		uint16_t mHeight = 1;
		uint16_t mDepth = 0;
	};
}
