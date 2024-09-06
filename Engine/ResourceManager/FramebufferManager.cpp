
#include "FramebufferManager.hpp"

#include "Util/Profiler.hpp"
#include "Util/ServiceLocator.hpp"
#include "Util/RenderThread.hpp"

#include <ext/imgui_incl.hpp>

#pragma optimize("", off)

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
		std::lock_guard<std::mutex> lock(mCacheMutex);
		if (!mCache.contains(id.mHandle)) {
			NEO_LOG_W("Invalid resource requested! Did you check for validity?");
			mFallback->bind();
			return *mFallback;
		}
		else {
			auto& pfb = const_cast<PooledFramebuffer&>(mCache[id.mHandle]->mResource);
			if (pfb.mFrameCount < 5) {
				pfb.mFrameCount++;
			}
			pfb.mUsedThisFrame = true;
			return pfb.mFramebuffer;
		}
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
					TRACY_GPUN("FramebufferManager::Create");
					std::lock_guard<std::mutex> lock(mCacheMutex);
					mCache.load(item.mHandle.mHandle, item, textureManager);
				});
			}
		}

		// Destroy
		std::vector<entt::id_type> handleDiscardQueue;
		std::vector<Framebuffer> resourceDiscardQueue;
		{
			std::lock_guard<std::mutex> lock(mCacheMutex);
			for (auto [id, resource] : mCache) {
				if (!resource) {
					NEO_LOG_W("Uhh, framebuffer resource doesn't exist I guess");
					handleDiscardQueue.push_back(id);
					continue;
				}
				if (resource->mResource.mFrameCount == 0) {
					if (!resource->mResource.mExternallyOwned) {
						for (auto& texId : resource->mResource.mFramebuffer.mTextures) {
							textureManager.discard(texId);
						}
					}
					handleDiscardQueue.push_back(id);
					resourceDiscardQueue.push_back(resource->mResource.mFramebuffer);
				}
				else {
					if (resource->mResource.mUsedThisFrame) {
						resource->mResource.mUsedThisFrame = false;
					}
					else {
						resource->mResource.mFrameCount--;
					}
				}
			}
		}
		for (auto& id : handleDiscardQueue) {
			mCache.erase(id);
		}
		if (!resourceDiscardQueue.empty()) {
			renderThread.pushRenderFunc([this, queue = std::move(resourceDiscardQueue)]() {
				TRACY_GPUN("FramebufferManager::Destroy");
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
			for(auto [id, pfb] : mCache) {
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
						textureFunc(*texId, textureManager);
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
		{
			std::lock_guard<std::mutex> lock(mQueueMutex);
			mQueue.clear();
		}
		std::lock_guard<std::mutex> lock(mCacheMutex);
		for (auto [id, pfb] : mCache) {
			if (!pfb->mResource.mExternallyOwned) {
				for (auto& textureHandle : pfb->mResource.mFramebuffer.mTextures) {
					textureManager.discard(textureHandle);
				}
			}
			ServiceLocator<RenderThread>::value().pushRenderFunc([this, fb = pfb->mResource.mFramebuffer]() {
				fb.destroy();
			});
		}
		mCache.clear();
	}

	std::shared_ptr<BackedResource<PooledFramebuffer>> FramebufferLoader::operator()(const FramebufferQueueItem& details, const TextureManager& textureManager) const {
		NEO_ASSERT(ServiceLocator<RenderThread>::value().isRenderThread(), "Only call this from render thread");

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

}
