#include "TextureManager.hpp"

#include "Util/Profiler.hpp"

#include "Loader/Loader.hpp"
#include "Loader/STBIImageData.hpp"

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

			std::shared_ptr<BackedResource<Texture>> load(TextureFiles& fileDetails, const std::optional<std::string>& debugName) const {
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
				bool successfulFileLoad = true;
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
						successfulFileLoad |= false;
					}
				}

				if (successfulFileLoad) {
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

			std::shared_ptr<BackedResource<Texture>> load(TextureBuilder textureDetails, const std::optional<std::string>& debugName) const {
				std::shared_ptr<BackedResource<Texture>> textureResource = std::make_shared<BackedResource<Texture>>(textureDetails.mFormat, textureDetails.mDimensions, debugName, textureDetails.mData);
				textureResource->mDebugName = debugName;
				if (textureDetails.mFormat.mFilter.usesMipFilter() || textureDetails.mFormat.mMipCount > 1) {
					textureResource->mResource.genMips();
				}

				return textureResource;
			}
		};

	}

	TextureManager::TextureManager() {
		uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF, /**/ 0xFF, 0xFF, 0xFF, 0xFF,
		                   0xFF, 0xFF, 0xFF, 0xFF, /**/ 0x00, 0x00, 0x00, 0xFF
		};
		mFallback = TextureLoader{}.load(TextureBuilder{
			TextureFormat{},
			glm::u16vec3(2, 2, 0),
			data
		}, "Fallback Texture");
	}

	TextureManager::~TextureManager() {
		mFallback->mResource.destroy();
		mFallback.reset();
	}

	[[nodiscard]] TextureHandle TextureManager::_asyncLoadImpl(TextureHandle id, TextureLoadDetails textureDetails, const std::optional<std::string>& debugName) const {
		NEO_UNUSED(debugName);
		std::visit(util::VisitOverloaded{
			[&](TextureBuilder& builder) {
				TextureBuilder copy = builder;
				if (builder.mData != nullptr) {
					// Base dimension
					uint32_t byteSize = glm::max<glm::u16>(builder.mDimensions.x, 1u) * glm::max<glm::u16>(builder.mDimensions.y, 1u) * glm::max<glm::u16>(builder.mDimensions.z, 1u);
					// Components per pixel
					byteSize *= _channelsPerPixel(TextureFormat::deriveBaseFormat(builder.mFormat.mInternalFormat));
					// Pixel format
					byteSize *= _bytesPerPixel(builder.mFormat.mType);

					copy.mData = static_cast<uint8_t*>(malloc(byteSize));
					memcpy(const_cast<uint8_t*>(copy.mData), builder.mData, byteSize);
				}
				mQueue.emplace_back(ResourceLoadDetails_Internal{ id, copy, debugName });
			},
			[&](TextureFiles& loadDetails) {
				NEO_ASSERT(loadDetails.mFilePaths.size() == 1 || loadDetails.mFilePaths.size() == 6, "Invalid file path count when loading texture");

				// Can safely copy into queue
				mQueue.emplace_back(ResourceLoadDetails_Internal{ id, loadDetails, debugName });
			},
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
		}, textureDetails);

		return id;
	}


	void TextureManager::_tickImpl() {
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
					else if constexpr (std::is_same_v<T, TextureFiles>) {
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

	void TextureManager::_destroyImpl(BackedResource<Texture>& texture) {
		texture.mResource.destroy();
	}

	void TextureManager::imguiEditor(std::function<void(const Texture&)> textureFunc) {
		mCache.each([&](auto handle, BackedResource<Texture>& textureResource) {
			ImGui::PushID(static_cast<int>(handle));
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
			ImGui::PopID();
		});
	}



}