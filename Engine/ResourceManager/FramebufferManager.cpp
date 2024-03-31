
#include "FramebufferManager.hpp"

#include "Util/Profiler.hpp"

#include <imgui.h>

namespace neo {
	namespace {
		TextureHandle swizzleTextureId(HashedString::hash_type srcId, TextureFormat format, glm::uvec2 dimension) {
			HashedString::hash_type seed = srcId ^ dimension.x ^ dimension.y;
			seed ^= static_cast<uint32_t>(format.mInternalFormat) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mType) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mFilter.mMin) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mFilter.mMag) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mWrap.mS) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mWrap.mR) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}

		FramebufferHandle swizzleSrcId(HashedString id, FramebufferLoadDetails& loadDetails) {
			HashedString::hash_type seed = id;
			std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, FramebufferBuilder>) {
					for (auto& format : arg.mFormats) {
						seed = swizzleTextureId(seed, format, arg.mSize);
					}
				}
				else if constexpr (std::is_same_v<T, std::vector<TextureHandle>>) {
					for (auto& handle : arg) {
						seed ^= static_cast<uint32_t>(handle) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
					}
				}
				else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, loadDetails);

			return seed;
		}

		struct FramebufferLoader final : entt::resource_loader<FramebufferLoader, PooledFramebuffer> {

			std::shared_ptr<PooledFramebuffer> load(const FramebufferQueueItem& details, const TextureResourceManager& manager) const {
				std::shared_ptr<PooledFramebuffer> framebuffer = std::make_shared<PooledFramebuffer>();
				framebuffer->mFramebuffer.init();
				framebuffer->mFrameCount = 1;
				framebuffer->mFullyOwned = details.second;
				for (auto& texHandle : details.first) {
					framebuffer->mFramebuffer.attachTexture(texHandle, manager.resolve(texHandle));
				}
				if (framebuffer->mFramebuffer.mColorAttachments) {
					framebuffer->mFramebuffer.initDrawBuffers();
				}

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

		mQueue.emplace(dstId, std::make_pair(texIds, owned));

		return dstId;
	}

	Framebuffer& FramebufferResourceManager::_resolveFinal(FramebufferHandle id) const {
		auto handle = mCache.handle(id);
		if (handle) {
			auto& pfb = const_cast<PooledFramebuffer&>(handle.get());
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
		std::map<FramebufferHandle, FramebufferQueueItem> swapQueue = {};
		std::swap(mQueue, swapQueue);
		mQueue.clear();
		for (auto&& [id, texHandles] : swapQueue) {
			mCache.load<FramebufferLoader>(id, texHandles, textureManager);
		}

		// Discard queue
		std::vector<FramebufferHandle> discardQueue;
		mCache.each([&](FramebufferHandle id, PooledFramebuffer& pfb) {
			if (pfb.mFrameCount == 0) {
				discardQueue.emplace_back(id);
				if (!pfb.mFullyOwned) {
					for (auto& texId : pfb.mFramebuffer.mTextures) {
						textureManager.discard(texId);
					}
				}
				pfb.mFramebuffer.destroy();
			}
			else {
				if (pfb.mUsedThisFrame) {
					pfb.mUsedThisFrame = false;
				}
				else {
					pfb.mFrameCount--;
				}
			}
		});
		for (auto& discardId : discardQueue) {
			mCache.discard(discardId);
		}
	}

	void FramebufferResourceManager::imguiEditor(std::function<void(Texture&)> textureFunc, TextureResourceManager& textureManager) {
		if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableSetupColumn("Name/Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending);
			ImGui::TableSetupColumn("Attachments");
			ImGui::TableHeadersRow();
			mCache.each([&](PooledFramebuffer& pfb) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(pfb.mFullyOwned ? "%s" : "*%s", pfb.mName.has_value() ? pfb.mName->c_str() : "");
				if (textureManager.isValid(pfb.mFramebuffer.mTextures[0])) {
					auto& firstTex = textureManager.resolve(pfb.mFramebuffer.mTextures[0]);
					ImGui::Text("[%d, %d]", firstTex.mWidth, firstTex.mHeight);
				}
				ImGui::TableSetColumnIndex(1);
				for (auto texId = pfb.mFramebuffer.mTextures.begin(); texId < pfb.mFramebuffer.mTextures.end(); texId++) {
					if (textureManager.isValid(*texId)) {
						textureFunc(textureManager.resolve(*texId));
						if (texId != std::prev(pfb.mFramebuffer.mTextures.end())) {
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
		mCache.each([&textureManager](PooledFramebuffer& framebuffer) {
			if (framebuffer.mFullyOwned) {
				for (auto& textureHandle : framebuffer.mFramebuffer.mTextures) {
					textureManager.discard(textureHandle);
				}
			}
			framebuffer.mFramebuffer.destroy();
		});
		mCache.clear();
	}
}
