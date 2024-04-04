
#include "FramebufferManager.hpp"

#include "Util/Profiler.hpp"

#include <imgui.h>

namespace neo {
	namespace {
		TextureHandle swizzleTextureId(FramebufferHandle srcHandle, TextureFormat format, glm::uvec2 dimension) {
			HashedString::hash_type seed = srcHandle.mHandle ^ dimension.x ^ dimension.y;
			seed ^= static_cast<uint32_t>(format.mInternalFormat) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mType) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mFilter.mMin) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mFilter.mMag) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mWrap.mS) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mWrap.mR) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return TextureHandle(seed);
		}

		FramebufferHandle swizzleSrcId(HashedString id, FramebufferLoadDetails& loadDetails) {
			HashedString::hash_type seed = id;
			std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, FramebufferBuilder>) {
					for (auto& format : arg.mFormats) {
						seed = TextureHandle(swizzleTextureId(seed, format, arg.mSize)).mHandle;
					}
				}
				else if constexpr (std::is_same_v<T, std::vector<TextureHandle>>) {
					for (auto& handle : arg) {
						seed ^= static_cast<uint32_t>(handle.mHandle) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
					}
				}
				else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, loadDetails);

			return FramebufferHandle(seed);
		}

		struct FramebufferLoader final : entt::resource_loader<FramebufferLoader, BackedResource<PooledFramebuffer>> {

			std::shared_ptr<BackedResource<PooledFramebuffer>> load(const FramebufferQueueItem& details, const TextureResourceManager& manager) const {
				std::shared_ptr<BackedResource<PooledFramebuffer>> framebuffer = std::make_shared<BackedResource<PooledFramebuffer>>();
				framebuffer->mResource.mFramebuffer.init(details.mDebugName);
				framebuffer->mResource.mFrameCount = 1;
				framebuffer->mResource.mExternallyOwned = details.mExternallyOwned;
				for (auto& texHandle : details.mTexIDs) {
					framebuffer->mResource.mFramebuffer.attachTexture(texHandle, manager.resolve(texHandle));
				}
				if (framebuffer->mResource.mFramebuffer.mColorAttachments) {
					framebuffer->mResource.mFramebuffer.initDrawBuffers();
				}
				framebuffer->mDebugName = details.mDebugName;

				return framebuffer;
			}
		};
	}

	FramebufferResourceManager::FramebufferResourceManager() {
		// Fallback is back buffer
		// Goodluck
		mFallback = std::make_shared<Framebuffer>();
	}

	FramebufferResourceManager::~FramebufferResourceManager() {
		mFallback.reset();
	}

	[[nodiscard]] FramebufferHandle FramebufferResourceManager::asyncLoad(const TextureResourceManager& textureManager, HashedString id, FramebufferLoadDetails framebufferDetails) const {
		FramebufferHandle dstId = swizzleSrcId(id, framebufferDetails);
		if (isValid(dstId) || isQueued(dstId)) {
			return dstId;
		}

		std::vector<TextureHandle> texIds;
		bool owned = false;
		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, FramebufferBuilder>) {
				for (auto& format : arg.mFormats) {
					texIds.emplace_back(textureManager.asyncLoad(swizzleTextureId(dstId, format, arg.mSize), TextureBuilder{ format, glm::u16vec3(arg.mSize, 0.0), nullptr }));
				}
			}
			else if constexpr (std::is_same_v<T, std::vector<TextureHandle>>) {
				texIds = arg;
				owned = true;
			}
			else {
				static_assert(always_false_v<T>, "non-exhaustive visitor!");
			}
			}, framebufferDetails);

		mQueue.emplace_back(FramebufferQueueItem{
			dstId, 
			texIds,
			owned,
			std::string(id)
		});

		return dstId;
	}

	Framebuffer& FramebufferResourceManager::_resolveFinal(FramebufferHandle id) const {
		auto handle = mCache.handle(id.mHandle);
		if (handle) {
			auto& pfb = const_cast<PooledFramebuffer&>(handle.get().mResource);
			if (pfb.mFrameCount < 5) {
				pfb.mFrameCount++;
			}
			pfb.mUsedThisFrame = true;
			pfb.mFramebuffer.bind();
			return pfb.mFramebuffer;
		}
		NEO_LOG_W("Invalid resource requested! Did you check for validity?");
		mFallback->bind();
		return *mFallback;
	}

	void FramebufferResourceManager::tick(const TextureResourceManager& textureManager) {
		TRACY_ZONE();

		// Create queue
		std::vector<FramebufferQueueItem> swapQueue = {};
		std::swap(mQueue, swapQueue);
		mQueue.clear();
		for (auto& item : swapQueue) {
			bool validTextures = true;
			for (auto& texId : item.mTexIDs) {
				if (!textureManager.isValid(texId)) {
					NEO_LOG_W("Trying to create a framebuffer %s with invalid texture attachments -- skipping", item.mDebugName.value_or("").c_str());
					validTextures = false;
					break;
				}
			}
			if (validTextures) {
				mCache.load<FramebufferLoader>(item.mHandle.mHandle, item, textureManager);
			}
			else {
				mQueue.emplace_back(item);
			}
		}

		// Discard queue
		std::vector<FramebufferHandle> discardQueue;
		mCache.each([&](const auto id, BackedResource<PooledFramebuffer>& pfb) {
			if (pfb.mResource.mFrameCount == 0) {
				discardQueue.emplace_back(id);
				if (!pfb.mResource.mExternallyOwned) {
					for (auto& texId : pfb.mResource.mFramebuffer.mTextures) {
						textureManager.discard(texId);
					}
				}
				pfb.mResource.mFramebuffer.destroy();
			}
			else {
				if (pfb.mResource.mUsedThisFrame) {
					pfb.mResource.mUsedThisFrame = false;
				}
				else {
					pfb.mResource.mFrameCount--;
				}
			}
		});
		for (auto& discardId : discardQueue) {
			mCache.discard(discardId.mHandle);
		}
	}

	void FramebufferResourceManager::imguiEditor(std::function<void(Texture&)> textureFunc, TextureResourceManager& textureManager) {
		if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableSetupColumn("Name/Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending);
			ImGui::TableSetupColumn("Attachments");
			ImGui::TableHeadersRow();
			mCache.each([&](auto id, BackedResource<PooledFramebuffer>& pfb) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (pfb.mDebugName.has_value()) {
					ImGui::Text(pfb.mResource.mExternallyOwned ? "%s" : "*%s", pfb.mDebugName->c_str());
				}
				else {
					ImGui::Text(pfb.mResource.mExternallyOwned ? "%d" : "*%d", id);
				}
				if (textureManager.isValid(pfb.mResource.mFramebuffer.mTextures[0])) {
					auto& firstTex = textureManager.resolve(pfb.mResource.mFramebuffer.mTextures[0]);
					ImGui::Text("[%d, %d]", firstTex.mWidth, firstTex.mHeight);
				}
				ImGui::TableSetColumnIndex(1);
				for (auto texId = pfb.mResource.mFramebuffer.mTextures.begin(); texId < pfb.mResource.mFramebuffer.mTextures.end(); texId++) {
					if (textureManager.isValid(*texId)) {
						textureFunc(textureManager.resolve(*texId));
						if (texId != std::prev(pfb.mResource.mFramebuffer.mTextures.end())) {
							ImGui::SameLine();
						}
					}
				}
			});
			ImGui::EndTable();
		}
	}

	void FramebufferResourceManager::clear(const TextureResourceManager& textureManager) {
		mQueue.clear();
		mCache.each([&textureManager](BackedResource<PooledFramebuffer>& pfb) {
			if (pfb.mResource.mExternallyOwned) {
				for (auto& textureHandle : pfb.mResource.mFramebuffer.mTextures) {
					textureManager.discard(textureHandle);
				}
			}
			pfb.mResource.mFramebuffer.destroy();
		});
		mCache.clear();
	}
}
