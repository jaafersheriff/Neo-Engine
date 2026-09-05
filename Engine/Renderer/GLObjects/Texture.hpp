#pragma once

#include "Renderer/Types.hpp"

#include <glm/glm.hpp>

namespace neo {

	struct TextureFilter {
		types::texture::Filters mMin = types::texture::Filters::Linear;
		types::texture::Filters mMag = types::texture::Filters::Linear;
		types::texture::Filters mMip = types::texture::Filters::Linear;

		bool operator==(const TextureFilter& other) const noexcept {
			return mMin == other.mMin
				&& mMag == other.mMag
				&& mMip == other.mMip;
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
		types::texture::InternalFormats mInternalFormat = types::texture::InternalFormats::RGBA8_UNORM;
		TextureFilter mFilter = {
			types::texture::Filters::Linear,
			types::texture::Filters::Linear,
			types::texture::Filters::Linear
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

	// Everything about a texture except the one field the driver cares about. A base rather than a
	// separate struct with copies of the fields, so that Texture::mWidth and friends still name this
	// storage - no call site changes, and no second copy that can drift out of step.
	//
	// The split is along the line that matters: a texture's shape is ordinary CPU data, mTextureID is
	// GL state. Anything that only needs the shape can hold this by value and never touch the live
	// object, which is what lets such a caller run somewhere other than the render thread.
	struct TextureDescriptor {
		TextureFormat mFormat;

		uint16_t mWidth = 1;
		uint16_t mHeight = 1;
		uint16_t mDepth = 0;
	};

	class Texture : public TextureDescriptor {
	public:

		Texture() = default;
		Texture(TextureFormat format,     uint16_t dimension, const std::optional<std::string>& debugName, const void* data = nullptr);
		Texture(TextureFormat format, glm::u16vec2 dimension, const std::optional<std::string>& debugName, const void* data = nullptr);
		Texture(TextureFormat format, glm::u16vec3 dimension, const std::optional<std::string>& debugName, const void* data = nullptr);

		void bind() const;
		void genMips();
		void destroy();

		// Deliberately a slice - the description is exactly this object minus the GL name.
		[[nodiscard]] TextureDescriptor getDescriptor() const { return *this; }

		uint32_t mTextureID = 0;
	};
}
