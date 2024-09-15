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

		bool usesMipFilter() const {
			NEO_ASSERT(mMag == types::texture::Filters::Nearest || mMag == types::texture::Filters::Linear, "These are the only allowed mag filter types");
			return mMin == types::texture::Filters::LinearMipmapLinear
				|| mMin == types::texture::Filters::LinearMipmapNearest
				|| mMin == types::texture::Filters::NearestMipmapLinear
				|| mMin == types::texture::Filters::NearestMipmapNearest
			;
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
	static TextureWrap ClampWrap = TextureWrap{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp };

	struct TextureFormat {
		types::texture::Target mTarget = types::texture::Target::Texture2D;
		types::texture::InternalFormats mInternalFormat = types::texture::InternalFormats::RGBA8_UNORM;
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
		uint16_t mMipCount = 1;

		static types::texture::BaseFormats deriveBaseFormat(types::texture::InternalFormats format);

		bool operator==(const TextureFormat& other) const noexcept {
			return mTarget == other.mTarget
				&& mInternalFormat == other.mInternalFormat
				&& mFilter == other.mFilter
				&& mWrap == other.mWrap
				&& mType == other.mType
				&& mMipCount == other.mMipCount
			;
		}
	};

	class Texture {
	public:

		Texture() = default;
		Texture(TextureFormat format,     uint16_t dimension, const std::optional<std::string>& debugName, const void* data = nullptr);
		Texture(TextureFormat format, glm::u16vec2 dimension, const std::optional<std::string>& debugName, const void* data = nullptr);
		Texture(TextureFormat format, glm::u16vec3 dimension, const std::optional<std::string>& debugName, const void* data = nullptr);

		void bind() const;
		void genMips();
		void destroy();

		uint32_t mTextureID = 0;
		TextureFormat mFormat;

		uint16_t mWidth = 1;
		uint16_t mHeight = 1;
		uint16_t mDepth = 0;
	};
}
