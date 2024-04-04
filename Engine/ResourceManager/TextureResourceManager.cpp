#include "TextureResourceManager.hpp"

#include "Util/Profiler.hpp"

#include "Loader/Loader.hpp"

#pragma warning(push)
#include <stb_image.h>
#pragma warning(pop)

#include <imgui.h>

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

		struct TextureLoader final : entt::resource_loader<TextureLoader, BackedResource<Texture>> {

			std::shared_ptr<BackedResource<Texture>> load(FileLoadDetails& fileDetails, std::optional<std::string> debugName) const {
				if (fileDetails.mFilePaths.size() == 6 && fileDetails.mFormat.mTarget != types::texture::Target::TextureCube) {
					NEO_LOG_E("Cubemap format mismatch!");
					fileDetails.mFilePaths.erase(fileDetails.mFilePaths.begin(), fileDetails.mFilePaths.begin() + 5);
				}

				std::vector<std::unique_ptr<STBImageData>> images;

				for (auto& filePath : fileDetails.mFilePaths) {
					std::string _fileName = Loader::APP_RES_DIR + filePath;
					if (!util::fileExists(_fileName.c_str())) {
						_fileName = Loader::ENGINE_RES_DIR + filePath;
						NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file %s", filePath.c_str());
					}

					bool flip = fileDetails.mFormat.mTarget != types::texture::Target::TextureCube; // This might be really dumb
					images.push_back(std::make_unique<STBImageData>(_fileName.c_str(), TextureFormat::deriveBaseFormat(fileDetails.mFormat.mInternalFormat), flip));
				}

				std::vector<uint8_t*> data;
				glm::u16vec3 dimensions(UINT16_MAX);
				bool check = true;
				for (auto& image : images) {
					if (image) {
						NEO_LOG_I("Loaded image %s [%d, %d]", image->mFilePath.c_str(), image->mWidth, image->mHeight);
						dimensions.x = glm::min(dimensions.x, static_cast<uint16_t>(image->mWidth));
						dimensions.y = glm::min(dimensions.y, static_cast<uint16_t>(image->mHeight));

						// TODO - need to memcpy the data over lmao
						data.push_back(image->mData);
					}
					else {
						NEO_FAIL("Error reading texture file %s", image->mFilePath.c_str());
						check |= false;
					}
				}

				if (check) {
					TextureBuilder details;
					details.mFormat = fileDetails.mFormat;
					details.mDimensions = dimensions;
					if (fileDetails.mFilePaths.size() == 1) {
						details.mData = data[0];
					}
					else if (fileDetails.mFilePaths.size() == 6) {
						// HEH??
						details.mData = reinterpret_cast<uint8_t*>(data.data());
					}
					else {
						NEO_FAIL("How u do dis");
					}

					return load(details, debugName);
				}
				return nullptr;
			}

			std::shared_ptr<BackedResource<Texture>> load(TextureBuilder textureDetails, std::optional<std::string> debugName) const {
				std::shared_ptr<BackedResource<Texture>> textureResource = std::make_shared<BackedResource<Texture>>(textureDetails.mFormat, textureDetails.mDimensions, textureDetails.mData, debugName);
				textureResource->mDebugName = debugName;
				if (textureDetails.mFormat.mFilter.usesMipFilter()) {
					textureResource->mResource.genMips();
				}

				return textureResource;
			}
		};

	}

	STBImageData::STBImageData(const char* filePath, types::texture::BaseFormats baseFormat, bool flip) {
		mFilePath = filePath;
		stbi_set_flip_vertically_on_load(flip);
		int _components;
		mData = stbi_load(mFilePath.c_str(), &mWidth, &mHeight, &_components, baseFormat == types::texture::BaseFormats::RGBA ? STBI_rgb_alpha : STBI_rgb);
	}

	STBImageData::~STBImageData() {
		stbi_image_free(mData);
	}

	TextureResourceManager::TextureResourceManager() {
		uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF, /**/ 0xFF, 0xFF, 0xFF, 0xFF,
		                   0xFF, 0xFF, 0xFF, 0xFF, /**/ 0x00, 0x00, 0x00, 0xFF
		};
		mFallback = TextureLoader{}.load(TextureBuilder{
			TextureFormat{},
			glm::u16vec3(2, 2, 0),
			data
		}, "Fallback Texture");
	}

	TextureResourceManager::~TextureResourceManager() {
		mFallback->mResource.destroy();
		mFallback.reset();
	}

	[[nodiscard]] TextureHandle TextureResourceManager::_asyncLoadImpl(TextureHandle id, TextureLoadDetails textureDetails, std::optional<std::string> debugName) const {
		NEO_UNUSED(debugName);
		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, TextureBuilder>) {
				TextureBuilder copy = arg;
				if (arg.mData != nullptr) {
					// Base dimension
					uint32_t byteSize = glm::max<glm::u16>(arg.mDimensions.x, 1u) * glm::max<glm::u16>(arg.mDimensions.y, 1u) * glm::max<glm::u16>(arg.mDimensions.z, 1u);
					// Components per pixel
					byteSize *= _channelsPerPixel(TextureFormat::deriveBaseFormat(arg.mFormat.mInternalFormat));
					// Pixel format
					byteSize *= _bytesPerPixel(arg.mFormat.mType);

					copy.mData = static_cast<uint8_t*>(malloc(byteSize));
					memcpy(const_cast<uint8_t*>(copy.mData), arg.mData, byteSize);
				}
				mQueue.emplace_back(ResourceLoadDetails_Internal{ id, copy, debugName });
			}
			else if constexpr (std::is_same_v<T, FileLoadDetails>) {
				NEO_ASSERT(arg.mFilePaths.size() == 1 || arg.mFilePaths.size() == 6, "Invalid file path count when loading texture");

				// Can safely copy into queue
				mQueue.emplace_back(ResourceLoadDetails_Internal{ id, arg, debugName });
			}
			else {
				static_assert(always_false_v<T>, "non-exhaustive visitor!");
			}
		}, textureDetails);

		return id;
	}


	void TextureResourceManager::_tickImpl() {
		TRACY_ZONE();

		{
			std::vector<ResourceLoadDetails_Internal> swapQueue;
			std::swap(mQueue, swapQueue);
			mQueue.clear();

			for (auto& loadDetails : swapQueue) {
				std::visit([&](auto&& arg) {
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, TextureBuilder>) {
						mCache.load<TextureLoader>(loadDetails.mHandle.mHandle, arg, loadDetails.mDebugName);
						free(const_cast<uint8_t*>(arg.mData));
					}
					else if constexpr (std::is_same_v<T, FileLoadDetails>) {
						mCache.load<TextureLoader>(loadDetails.mHandle.mHandle, arg, loadDetails.mDebugName);
					}
					else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
					}, loadDetails.mLoadDetails);
			}
		}

		{
			std::vector<TextureHandle> swapQueue;
			std::swap(mDiscardQueue, swapQueue);
			mDiscardQueue.clear();

			for (auto& id : swapQueue) {
				if (isValid(id)) {
					mCache.handle(id.mHandle).get().mResource.destroy();
					mCache.discard(id.mHandle);
				}
			}
		}
	}

	void TextureResourceManager::_clearImpl() {
		mQueue.clear();
		mCache.each([](BackedResource<Texture>& texture) {
			texture.mResource.destroy();
		});
		mCache.clear();
	}

	void TextureResourceManager::imguiEditor(std::function<void(const Texture&)> textureFunc) {
		mCache.each([&](auto handle, BackedResource<Texture>& textureResource) {
			bool node = false;
			if (textureResource.mDebugName.has_value()) {
				node |= ImGui::TreeNode(static_cast<void*>(&handle), "%s", textureResource.mDebugName->c_str());
			}
			else {
				node |= ImGui::TreeNode(static_cast<void*>(&handle), "%d", handle);
			}
			if (node) {
				ImGui::Text("[%d, %d]", textureResource.mResource.mWidth, textureResource.mResource.mHeight);
				textureFunc(textureResource.mResource);
				ImGui::TreePop();
			}
		});
	}



}