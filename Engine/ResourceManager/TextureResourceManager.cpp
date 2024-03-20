#include "TextureResourceManager.hpp"

#include "Util/Profiler.hpp"

namespace neo {
	namespace {
		uint16_t _bytesPerPixel(types::ByteFormats format) {
			switch (format) {
			case types::ByteFormats::UnsignedByte:
			case types::ByteFormats::Byte:
				return 1;
			case types::ByteFormats::Short:
			case types::ByteFormats::UnsignedShort:
				return 2;
			case types::ByteFormats::Int:
			case types::ByteFormats::UnsignedInt:
			case types::ByteFormats::UnsignedInt24_8:
			case types::ByteFormats::Float:
				return 4;
			case types::ByteFormats::Double:
				return 8;
			default:
				NEO_FAIL("Invalid");
				return 1;
			}
		}

		uint16_t _channelsPerPixel(types::texture::BaseFormats format) {
			switch (format) {
			case types::texture::BaseFormats::R:
			case types::texture::BaseFormats::Depth:
			case types::texture::BaseFormats::DepthStencil:
				return 1;
			case types::texture::BaseFormats::RG:
				return 2;
			case types::texture::BaseFormats::RGB:
				return 3;
			case types::texture::BaseFormats::RGBA:
				return 4;
			default:
				NEO_FAIL("INVALID");
				return 1;
			}
		}
	}

	struct TextureLoader final : entt::resource_loader<TextureLoader, Texture> {

		std::shared_ptr<Texture> load(TextureResourceManager::TextureBuilder textureDetails) const {
			std::shared_ptr<Texture> texture = std::make_shared<Texture>(textureDetails.mFormat, textureDetails.mDimensions, textureDetails.data);
			if (textureDetails.mFormat.mFilter.usesMipFilter()) {
				texture->genMips();
			}

			return texture;
		}
	};

	bool TextureResourceManager::isValid(TextureHandle id) const {
		return mTextureCache.contains(id);
	}

	Texture& TextureResourceManager::get(HashedString id) {
		return get(id.value());
	}

	const Texture& TextureResourceManager::get(HashedString id) const {
		return get(id.value());
	}

	Texture& TextureResourceManager::get(TextureHandle id) {
		if (mTextureCache.contains(id)) {
			return mTextureCache.handle(id).get();
		}
		NEO_FAIL("Invalid mesh requested");
		return mDummyTexture;
	}

	const Texture& TextureResourceManager::get(TextureHandle id) const {
		if (mTextureCache.contains(id)) {
			return mTextureCache.handle(id).get();
		}
		NEO_FAIL("Invalid mesh requested");
		return mTextureCache.handle(HashedString("cube")).get();
	}

	[[nodiscard]] TextureHandle TextureResourceManager::asyncLoad(HashedString id, TextureBuilder& textureDetails) const {
		if (!isValid(id)) {
			TextureBuilder copy = textureDetails;
			// Base dimension
			uint32_t byteSize = glm::max<glm::u16>(textureDetails.mDimensions.x, 1u) * glm::max<glm::u16>(textureDetails.mDimensions.y, 1u) * glm::max<glm::u16>(textureDetails.mDimensions.z, 1u);
			// Components per pixel
			byteSize *= _channelsPerPixel(TextureFormat::deriveBaseFormat(textureDetails.mFormat.mInternalFormat));
			// Pixel format
			byteSize *= _bytesPerPixel(textureDetails.mFormat.mType);
			
			copy.data = static_cast<uint8_t*>(malloc(byteSize));
			memcpy(const_cast<uint8_t*>(copy.data), textureDetails.data, byteSize);
			mQueue.emplace_back(std::make_pair(id, copy));
		}

		return id;
	}

	void TextureResourceManager::_tick() {
		TRACY_ZONE();
		for (auto&& [id, meshDetails] : mQueue) {
			mTextureCache.load<TextureLoader>(id, meshDetails);
		}
		mQueue.clear();
	}

	void TextureResourceManager::clear() {
		mTextureCache.each([](Texture& mesh) {
			mesh.destroy();
		});
		mTextureCache.clear();
	}
}