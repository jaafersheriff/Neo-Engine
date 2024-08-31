#include "TextureManager.hpp"

#include "Util/Profiler.hpp"
#include "Util/RenderThread.hpp"
#include "Util/ServiceLocator.hpp"

#include "Loader/Loader.hpp"
#include "Loader/STBIImageData.hpp"

#include <ext/imgui_incl.hpp>

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

			std::shared_ptr<BackedResource<Texture>> load(TextureFiles fileDetails, const std::optional<std::string>& debugName) const {
				NEO_ASSERT(ServiceLocator<RenderThread>::ref().isRenderThread(), "Only call this from render thread");
				if (debugName.has_value()) {
					NEO_LOG_V("Uploading texture files for %s", debugName.value().c_str());
				}
				if (fileDetails.mFilePaths.size() == 6 && fileDetails.mFormat.mTarget != types::texture::Target::TextureCube) {
					NEO_LOG_E("Cubemap format mismatch!");
					fileDetails.mFilePaths.erase(fileDetails.mFilePaths.begin(), fileDetails.mFilePaths.begin() + 5);
				}

				std::vector<std::unique_ptr<STBImageData>> images;

				for (auto& filePath : fileDetails.mFilePaths) {
					std::string _fileName = Loader::APP_RES_DIR + filePath;
					if (!util::fileExists(_fileName.c_str())) {
						_fileName = Loader::ENGINE_RES_DIR + filePath;
						if (!util::fileExists(_fileName.c_str())) {
							NEO_LOG_E("Unable to find file %s", filePath.c_str());
							continue;
						}
					}

					bool flip = fileDetails.mFormat.mTarget != types::texture::Target::TextureCube; // This might be really dumb
					images.push_back(std::make_unique<STBImageData>(_fileName.c_str(), TextureFormat::deriveBaseFormat(fileDetails.mFormat.mInternalFormat), fileDetails.mFormat.mType, flip));
				}

				std::vector<uint8_t*> data;
				glm::u16vec3 dimensions(UINT16_MAX);
				bool successfulFileLoad = !images.empty();
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
				else {
					NEO_LOG_E("Failed to load %s", debugName.has_value() ? debugName.value().c_str() : "");
				}
				return nullptr;
			}

			std::shared_ptr<BackedResource<Texture>> load(TextureBuilder textureDetails, const std::optional<std::string>& debugName) const {
				NEO_ASSERT(ServiceLocator<RenderThread>::ref().isRenderThread(), "Only call this from render thread");
				if (debugName.has_value()) {
					NEO_LOG_V("Uploading raw texture %s", debugName.value().c_str());
				}
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
		ServiceLocator<RenderThread>::ref().pushRenderFunc([this]() {
			uint8_t data[] = { 0x00, 0x00, 0x00, 0xFF, /**/ 0xFF, 0xFF, 0xFF, 0xFF,
							   0xFF, 0xFF, 0xFF, 0xFF, /**/ 0x00, 0x00, 0x00, 0xFF
			};
			mFallback = TextureLoader{}.load(TextureBuilder{
				TextureFormat{},
				glm::u16vec3(2, 2, 0),
				data
				}, "Fallback Texture");
		});
	}

	TextureManager::~TextureManager() {
		mFallback->mResource.destroy();
		mFallback.reset();
	}

	[[nodiscard]] TextureHandle TextureManager::_asyncLoadImpl(TextureHandle id, TextureLoadDetails textureDetails, const std::optional<std::string>& debugName) const {
		NEO_UNUSED(debugName);
		util::visit(textureDetails,
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

				std::lock_guard<std::mutex> lock(mQueueMutex);
				mQueue.emplace_back(ResourceLoadDetails_Internal{ id, copy, debugName });
			},
			[&](TextureFiles& loadDetails) {
				NEO_ASSERT(loadDetails.mFilePaths.size() == 1 || loadDetails.mFilePaths.size() == 6, "Invalid file path count when loading texture");

				// Can safely copy into queue
				std::lock_guard<std::mutex> lock(mQueueMutex);
				mQueue.emplace_back(ResourceLoadDetails_Internal{ id, loadDetails, debugName });
			},
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
		);

		return id;
	}

	void TextureManager::_tickImpl(RenderThread& renderThread) {
		TRACY_ZONE();

		// Destroy
		{
			std::vector<TextureHandle> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mDiscardMutex);
				std::swap(mDiscardQueue, swapQueue);
				mDiscardQueue.clear();
			}
			for (auto& id : swapQueue) {
				bool foundInQueue = false;
				std::lock_guard<std::mutex> lock(mQueueMutex);
				for (int i = 0; i < mQueue.size(); i++) {
					if (id == mQueue[i].mHandle) {
						mQueue.erase(mQueue.begin() + i);
						foundInQueue = true;
						break;
					}
				}
				if (foundInQueue) {
					continue;
				}
				renderThread.pushRenderFunc([this, id]() {
					if (isValid(id)) {
						std::lock_guard<std::mutex> lock(mCacheMutex);
						_destroyImpl(mCache.handle(id.mHandle).get());
						mCache.discard(id.mHandle);
					}
				});
			}
		}

		// Create
		{
			std::vector<ResourceLoadDetails_Internal> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mQueueMutex);
				std::swap(mQueue, swapQueue);
				mQueue.clear();
			}

			for (auto& loadDetails : swapQueue) {
				util::visit(loadDetails.mLoadDetails,
					[&](TextureBuilder builder) {
						renderThread.pushRenderFunc([this, builder, loadDetails]() {
							{
								std::lock_guard<std::mutex> lock(mCacheMutex);
								mCache.load<TextureLoader>(loadDetails.mHandle.mHandle, builder, loadDetails.mDebugName);
							}
							free(const_cast<uint8_t*>(builder.mData));
						});
					},
					[&](TextureFiles files) {
						renderThread.pushRenderFunc([this, files, loadDetails]() {
							std::lock_guard<std::mutex> lock(mCacheMutex);
							mCache.load<TextureLoader>(loadDetails.mHandle.mHandle, files, loadDetails.mDebugName);
						});
					},
					[](auto) {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				);
			}
		}
	}

	void TextureManager::_destroyImpl(BackedResource<Texture>& texture) {
		NEO_ASSERT(ServiceLocator<RenderThread>::ref().isRenderThread(), "Only call this from render thread");
		texture.mResource.destroy();
	}

	void TextureManager::imguiEditor(std::function<void(TextureHandle&, TextureManager&)> textureFunc) {
		std::lock_guard<std::mutex> lock(mCacheMutex);
		mCache.each([&](TextureHandle handle) {
			ImGui::PushID(static_cast<int>(handle.mHandle));
			bool node = false;
			const BackedResource<Texture>& texture = mCache.handle(handle.mHandle).get();
			if (texture.mDebugName.has_value()) {
				node |= ImGui::TreeNode(static_cast<void*>(&handle), "%s", texture.mDebugName->c_str());
			}
			else {
				node |= ImGui::TreeNode(static_cast<void*>(&handle), "%d", handle);
			}
			if (node) {
				ImGui::Text("[%d, %d]", texture.mResource.mWidth, texture.mResource.mHeight);
				textureFunc(handle, *this);
				ImGui::TreePop();
			}
			ImGui::PopID();
		});
	}



}
