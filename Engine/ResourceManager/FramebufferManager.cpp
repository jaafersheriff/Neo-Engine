
#include "FramebufferManager.hpp"

#include "Util/Profiler.hpp"
#include "Util/ServiceLocator.hpp"
#include "Util/RenderThread.hpp"

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

		struct FramebufferLoader final : entt::resource_loader<FramebufferLoader, BackedResource<PooledFramebuffer>> {

			std::shared_ptr<BackedResource<PooledFramebuffer>> load(const FramebufferQueueItem& details, const TextureManager& textureManager) const {
				NEO_ASSERT(ServiceLocator<RenderThread>::ref().isRenderThread(), "Only call this from render thread");

				std::shared_ptr<BackedResource<PooledFramebuffer>> framebuffer = std::make_shared<BackedResource<PooledFramebuffer>>();
				framebuffer->mResource.mFramebuffer.init(details.mDebugName);
				framebuffer->mResource.mFrameCount = 4;
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
		};
	}

	FramebufferManager::FramebufferManager() {
		// Fallback is back buffer
		// Goodluck
		mFallback = std::make_shared<Framebuffer>();
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

		std::lock_guard<std::mutex> lock(mQueueMutex);
		mQueue.emplace_back(FramebufferQueueItem{
			dstId, 
			attachments,
			owned,
			std::string(id)
		});

		return dstId;
	}

	Framebuffer& FramebufferManager::_resolveFinal(FramebufferHandle id) const {
		entt::resource_handle<const BackedResource<PooledFramebuffer>> handle;
		{
			std::lock_guard<std::mutex> lock(mCacheMutex);
			handle = mCache.handle(id.mHandle);
		}
		if (handle) {
			auto& pfb = const_cast<PooledFramebuffer&>(handle.get().mResource);
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

	void FramebufferManager::tick(const TextureManager& textureManager, RenderThread& renderThread) {
		TRACY_ZONE();

		// Create
		std::vector<FramebufferQueueItem> swapQueue = {};
		{
			std::lock_guard<std::mutex> lock(mQueueMutex);
			std::swap(mQueue, swapQueue);
			mQueue.clear();
		}
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
				renderThread.pushRenderFunc([this, item, &textureManager]() {
					std::lock_guard<std::mutex> lock(mCacheMutex);
					mCache.load<FramebufferLoader>(item.mHandle.mHandle, item, textureManager);
				});
			}
		}

		// Destroy
		std::vector<Framebuffer> discardQueue;
		std::lock_guard<std::mutex> lock(mCacheMutex);
		mCache.each([&](const auto id, BackedResource<PooledFramebuffer>& pfb) {
			if (&pfb == nullptr) {
				NEO_LOG_W("PFB IS INVALUD THREADING ISSUE AHHHHHHHHHHHHH");
				return;
			}
			if (pfb.mResource.mFrameCount == 0) {
				if (!pfb.mResource.mExternallyOwned) {
					for (auto& texId : pfb.mResource.mFramebuffer.mTextures) {
						textureManager.discard(texId);
					}
				}
				discardQueue.push_back(pfb.mResource.mFramebuffer);
				mCache.discard(id);
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
		if (!discardQueue.empty()) {
			renderThread.pushRenderFunc([this, queue = std::move(discardQueue)]() {
				for (int i = 0; i < queue.size(); i++) {
					queue[i].destroy();
				}
			});
		}
	}

	void FramebufferManager::imguiEditor(std::function<void(TextureHandle&, TextureManager&)> textureFunc, TextureManager& textureManager) {
		if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableSetupColumn("Name/Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending);
			ImGui::TableSetupColumn("Attachments");
			ImGui::TableHeadersRow();
			std::lock_guard<std::mutex> lock(mCacheMutex);
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
						textureFunc(*texId, textureManager);
						if (texId != std::prev(pfb.mResource.mFramebuffer.mTextures.end())) {
							ImGui::SameLine();
						}
					}
				}
			});
			ImGui::EndTable();
		}
	}

	void FramebufferManager::clear(const TextureManager& textureManager) {
		{
			std::lock_guard<std::mutex> lock(mQueueMutex);
			mQueue.clear();
		}
		std::lock_guard<std::mutex> lock(mCacheMutex);
		mCache.each([this, &textureManager](BackedResource<PooledFramebuffer>& pfb) {
			if (!pfb.mResource.mExternallyOwned) {
				for (auto& textureHandle : pfb.mResource.mFramebuffer.mTextures) {
					textureManager.discard(textureHandle);
				}
			}
			ServiceLocator<RenderThread>::ref().pushRenderFunc([this, &pfb]() {
				_destroyImpl(pfb);
			});
		});
		mCache.clear();
	}

	void FramebufferManager::_destroyImpl(BackedResource<PooledFramebuffer>& pfb) {
		NEO_ASSERT(ServiceLocator<RenderThread>::ref().isRenderThread(), "Only call this from render thread");
		pfb.mResource.mFramebuffer.destroy();
	}
}
