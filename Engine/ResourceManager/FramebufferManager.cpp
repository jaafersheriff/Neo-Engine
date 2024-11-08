
#include "FramebufferManager.hpp"

#include "Util/Profiler.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {
	namespace {
		TextureHandle swizzleTextureId(FramebufferHandle srcHandle, TextureFormat format, types::framebuffer::AttachmentTarget target, uint8_t mip, glm::uvec2 dimension) {
			HashedString::hash_type seed = srcHandle.mHandle ^ dimension.x ^ dimension.y;
			seed ^= static_cast<uint32_t>(format.mInternalFormat) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mType) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mFilter.mMin) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mFilter.mMag) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mWrap.mS) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mWrap.mR) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(format.mMipCount) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(target) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= static_cast<uint32_t>(mip) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return TextureHandle(seed);
		}

		FramebufferHandle swizzleSrcId(HashedString id, FramebufferLoadDetails& loadDetails) {
			HashedString::hash_type seed = id;
			util::visit(loadDetails,
				[&](FramebufferBuilder& builder) {
					for (auto& attachment : builder.mAttachments) {
						seed ^= TextureHandle(swizzleTextureId(seed, attachment.mFormat, attachment.mTarget, attachment.mMip, builder.mSize)).mHandle;
					}
				},
				[&](FramebufferExternalAttachments& externalHandles) {
					for (auto& handle : externalHandles) {
						seed ^= static_cast<uint32_t>(handle.mHandle.mHandle) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
						seed ^= static_cast<uint32_t>(handle.mTarget) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
						seed ^= static_cast<uint32_t>(handle.mMip) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
					}
				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);

			return FramebufferHandle(seed);
		}
	}

	FramebufferManager::FramebufferManager() {
		// Fallback is back buffer
		// Goodluck
		mFallback = std::make_shared<Framebuffer>();
		mBackbuffer = Framebuffer{};
	}

	FramebufferManager::~FramebufferManager() {
		mFallback.reset();
	}

	[[nodiscard]] FramebufferHandle FramebufferManager::asyncLoad(HashedString id, FramebufferLoadDetails framebufferDetails, const TextureManager& textureManager) const {
		FramebufferHandle dstId = swizzleSrcId(id, framebufferDetails);
		if (isValid(dstId) || isQueued(dstId)) {
			return dstId;
		}

		std::vector<FramebufferAttachment> attachments;
		bool owned = false;
		util::visit(framebufferDetails,
			[&](FramebufferBuilder& builder) {
				for (int i = 0; i < builder.mAttachments.size(); i++) {
					auto& attachment = builder.mAttachments[i];
					attachments.emplace_back(FramebufferAttachment{
						textureManager.asyncLoad(
							swizzleTextureId(dstId.mHandle + i, attachment.mFormat, attachment.mTarget, attachment.mMip, builder.mSize), 
							TextureBuilder{ attachment.mFormat, glm::u16vec3(builder.mSize, 0.0)}, std::string(id.data()) + "_" + std::to_string(i)),
						attachment.mTarget,
						attachment.mMip
					});
				}
			},
			[&](FramebufferExternalAttachments& externalAttachments) {
				attachments = externalAttachments;
				owned = true;
			},
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
		);

		mQueue.emplace_back(FramebufferQueueItem{
			dstId, 
			attachments,
			owned,
			std::string(id)
		});

		return dstId;
	}

	Framebuffer& FramebufferManager::_resolveFinal(FramebufferHandle id) const {
		// 0 is backbuffer, goodluck
		if (id == 0) {
			return mBackbuffer;
		}
		if (mCache.contains(id.mHandle)) {
			auto& pfb = const_cast<PooledFramebuffer&>(mCache[id.mHandle]->mResource);
			if (pfb.mFrameCount < 5) {
				pfb.mFrameCount++;
			}
			pfb.mUsedThisFrame = true;
			//pfb.mFramebuffer.bind();
			return pfb.mFramebuffer;
		}
		NEO_LOG_W("Invalid resource requested! Did you check for validity?");
		mFallback->bind();
		return *mFallback;
	}

	void FramebufferManager::tick(const TextureManager& textureManager) {
		TRACY_ZONE();

		// Create queue
		std::vector<FramebufferQueueItem> swapQueue = {};
		std::swap(mQueue, swapQueue);
		mQueue.clear();
		for (auto& item : swapQueue) {
			bool validTextures = true;
			for (auto& attachment : item.mAttachments) {
				if (!textureManager.isValid(attachment.mHandle)) {
					NEO_LOG_W("Trying to create a framebuffer %s with invalid texture attachments -- skipping", item.mDebugName.value_or("").c_str());
					validTextures = false;
					break;
				}
				else {
					auto& texture = textureManager.resolve(attachment.mHandle);
					if (texture.mFormat.mTarget == types::texture::Target::Texture2D && attachment.mTarget != types::framebuffer::AttachmentTarget::Target2D) {
						NEO_LOG_E("Trying to bind non-2D target to a 2D texture");
						validTextures = false;
						break;
					}
				}
			}
			if (validTextures) {
				mCache.load(item.mHandle.mHandle, item, textureManager);
			}
		}

		// Discard queue
		std::vector<FramebufferHandle> discardQueue;
		for (auto&& [id, pfb] : mCache) {
			if (pfb->mResource.mFrameCount == 0) {
				discardQueue.emplace_back(id);
				if (!pfb->mResource.mExternallyOwned) {
					for (auto& texId : pfb->mResource.mFramebuffer.mTextures) {
						textureManager.discard(texId);
					}
				}
				pfb->mResource.mFramebuffer.destroy();
			}
			else {
				if (pfb->mResource.mUsedThisFrame) {
					pfb->mResource.mUsedThisFrame = false;
				}
				else {
					pfb->mResource.mFrameCount--;
				}
			}
		}
		for (auto& discardId : discardQueue) {
			mCache.erase(discardId.mHandle);
		}
	}

	void FramebufferManager::imguiEditor(std::function<void(const TextureHandle&)> textureFunc, TextureManager& textureManager) {
		if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableSetupColumn("Name/Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending);
			ImGui::TableSetupColumn("Attachments");
			ImGui::TableHeadersRow();
			for(auto&& [id, pfb] : mCache) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (pfb->mDebugName.has_value()) {
					ImGui::Text(pfb->mResource.mExternallyOwned ? "%s" : "*%s", pfb->mDebugName->c_str());
				}
				else {
					ImGui::Text(pfb->mResource.mExternallyOwned ? "%d" : "*%d", id);
				}
				if (textureManager.isValid(pfb->mResource.mFramebuffer.mTextures[0])) {
					auto& firstTex = textureManager.resolve(pfb->mResource.mFramebuffer.mTextures[0]);
					ImGui::Text("[%d, %d]", firstTex.mWidth, firstTex.mHeight);
				}
				ImGui::TableSetColumnIndex(1);
				for (auto texId = pfb->mResource.mFramebuffer.mTextures.begin(); texId < pfb->mResource.mFramebuffer.mTextures.end(); texId++) {
					if (textureManager.isValid(*texId)) {
						textureFunc(*texId);
						if (texId != std::prev(pfb->mResource.mFramebuffer.mTextures.end())) {
							ImGui::SameLine();
						}
					}
				}
			}
			ImGui::EndTable();
		}
	}

	void FramebufferManager::clear(const TextureManager& textureManager) {
		mQueue.clear();
		for(auto&& [_, pfb] : mCache) {
			if (!pfb->mResource.mExternallyOwned) {
				for (auto& textureHandle : pfb->mResource.mFramebuffer.mTextures) {
					textureManager.discard(textureHandle);
				}
			}
			pfb->mResource.mFramebuffer.destroy();
		}
		mCache.clear();
	}

	FramebufferManager::FramebufferLoader::result_type FramebufferManager::FramebufferLoader::operator()(const FramebufferQueueItem& details, const TextureManager& textureManager) const {
		result_type framebuffer = std::make_shared<BackedResource<PooledFramebuffer>>();
		framebuffer->mResource.mFramebuffer.init(details.mDebugName);
		framebuffer->mResource.mFrameCount = 1;
		framebuffer->mResource.mExternallyOwned = details.mExternallyOwned;
		for (auto& attachment : details.mAttachments) {
			framebuffer->mResource.mFramebuffer.attachTexture(attachment.mHandle, textureManager.resolve(attachment.mHandle), attachment.mTarget, attachment.mMip);
		}
		if (framebuffer->mResource.mFramebuffer.mColorAttachments) {
			framebuffer->mResource.mFramebuffer.initDrawBuffers();
		}
		framebuffer->mDebugName = details.mDebugName;

		return framebuffer;
	}
}
