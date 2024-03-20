#include "TextureResourceManager.hpp"

#include "Util/Profiler.hpp"

#include "Loader/Loader.hpp"

#pragma warning(push)
#include <stb_image.h>
#pragma warning(pop)

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

		struct STBImageData {
			STBImageData(const char* _filePath, types::texture::BaseFormats baseFormat, bool flip) {
				filePath = _filePath;
				stbi_set_flip_vertically_on_load(flip);
				int components;
				data = stbi_load(filePath, &width, &height, &components, baseFormat == types::texture::BaseFormats::RGBA ? STBI_rgb_alpha : STBI_rgb);
			}

			~STBImageData() {
				stbi_image_free(data);
			}

			operator bool() const {
				return data != nullptr;
			}

			const char* filePath;
			uint8_t* data = nullptr;
			int width = 0;
			int height = 0;
		};

		struct TextureLoader final : entt::resource_loader<TextureLoader, Texture> {

			std::shared_ptr<Texture> load(std::vector<std::string>& filePaths, TextureFormat format, std::shared_ptr<Texture> dummy) const {
				if (filePaths.size() == 6 && format.mTarget != types::texture::Target::TextureCube) {
					NEO_LOG_E("Cubemap format mismatch!");
					filePaths.erase(filePaths.begin(), filePaths.begin() + 5);
				}

				std::vector<std::unique_ptr<STBImageData>> images;

				for (auto& filePath : filePaths) {
					std::string _fileName = Loader::APP_RES_DIR + filePath;
					if (!util::fileExists(_fileName.c_str())) {
						_fileName = Loader::ENGINE_RES_DIR + filePath;
						NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file %s", filePath.c_str());
					}

					bool flip = format.mTarget != types::texture::Target::TextureCube; // This might be really dumb
					images.push_back(std::make_unique<STBImageData>(_fileName.c_str(), TextureFormat::deriveBaseFormat(format.mInternalFormat), flip));
				}

				std::vector<uint8_t*> data;
				glm::u16vec3 dimensions(UINT16_MAX);
				bool check = true;
				for (auto& image : images) {
					if (image) {
						NEO_LOG_I("Loaded texture %s [%d, %d]", image->filePath, image->width, image->height);
						data.push_back(image->data);
						dimensions.x = glm::min(dimensions.x, static_cast<uint16_t>(image->width));
						dimensions.y = glm::min(dimensions.y, static_cast<uint16_t>(image->height));
					}
					else {
						NEO_FAIL("Error reading texture file %s", image->filePath);
						check |= false;
					}
				}

				if (check) {
					TextureResourceManager::TextureBuilder details;
					details.mFormat = format;
					details.mDimensions = dimensions;
					// HEH?
					details.data = reinterpret_cast<uint8_t*>(data.data());

					auto texture = load(details);
					return texture;
				}
				return dummy;
			}

			std::shared_ptr<Texture> load(TextureResourceManager::TextureBuilder textureDetails) const {
				std::shared_ptr<Texture> texture = std::make_shared<Texture>(textureDetails.mFormat, textureDetails.mDimensions, textureDetails.data);
				if (textureDetails.mFormat.mFilter.usesMipFilter()) {
					texture->genMips();
				}

				return texture;
			}
		};
	}

	TextureResourceManager::TextureResourceManager() {
		uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF, /**/ 0xFF, 0xFF, 0xFF, 0xFF,
		                   0xFF, 0xFF, 0xFF, 0xFF, /**/ 0x00, 0x00, 0x00, 0xFF
		};
		mDummyTexture = std::make_shared<Texture>(TextureFormat{}, glm::u16vec2(2, 2), data);
	}

	TextureResourceManager::~TextureResourceManager() {
		mDummyTexture->destroy();
		mDummyTexture.reset();
	}

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
		return *mDummyTexture;
	}

	const Texture& TextureResourceManager::get(TextureHandle id) const {
		if (mTextureCache.contains(id)) {
			return mTextureCache.handle(id).get();
		}
		NEO_FAIL("Invalid mesh requested");
		return mTextureCache.handle(HashedString("cube")).get();
	}

	[[nodiscard]] TextureHandle TextureResourceManager::asyncLoad(const char* filePath, TextureFormat format) const {
		return asyncLoad(filePath, { filePath }, format);
	}

	[[nodiscard]] TextureHandle TextureResourceManager::asyncLoad(const char* name, std::vector<std::string> filePath, TextureFormat format) const {
		NEO_ASSERT(filePath.size() == 1 || filePath.size() == 6, "Invalid file path count when loading texture");

		HashedString id(name);
		if (!isValid(id)) {
			mFileLoadQueue.push_back(std::make_pair(id, std::make_pair(filePath, format)));
		}

		return id;
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
		for (auto&& [id, pathsAndFormat] : mFileLoadQueue) {
			mTextureCache.load<TextureLoader>(id, pathsAndFormat.first, pathsAndFormat.second, mDummyTexture);
		}
		for (auto&& [id, textureDetails] : mQueue) {
			mTextureCache.load<TextureLoader>(id, textureDetails);
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