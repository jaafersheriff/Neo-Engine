
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

		FramebufferHandle swizzleSrcId(HashedString id, FramebufferLoadDetails& loadDetails, const TextureManager& textureManager) {
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
						if (textureManager.isValid(handle.mHandle)) {
							seed ^= static_cast<uint32_t>(textureManager.resolve(handle.mHandle).mTextureID) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
							seed ^= static_cast<uint32_t>(textureManager.getTimeStamp(handle.mHandle)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
						}
					}
				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);

			return FramebufferHandle(seed);
		}

		struct FramebufferLoader final {

			std::optional<CachedResource<ManagedFramebuffer>> load(const FramebufferAttachments& attachments, const std::optional<std::string>& debugName, const TextureManager& textureManager) const {
				for (auto& attachment : attachments.mAttachments) {
					if (!textureManager.isValid(attachment.mHandle)) {
						NEO_LOG_W("Trying to create a framebuffer %s with invalid texture attachments -- skipping", debugName.value_or("").c_str());
						return std::nullopt;
					}
					const Texture& texture = textureManager.resolve(attachment.mHandle);
					if (texture.mFormat.mTarget == types::texture::Target::Texture2D && attachment.mTarget != types::framebuffer::AttachmentTarget::Target2D) {
						NEO_LOG_E("Trying to bind non-2D target to a 2D texture");
						return std::nullopt;
					}
				}

				CachedResource<ManagedFramebuffer> framebuffer;
				framebuffer.mResource.mFramebuffer.init(debugName);
				framebuffer.mResource.mExternallyOwned = attachments.mExternallyOwned;
				for (auto& attachment : attachments.mAttachments) {
					framebuffer.mResource.mFramebuffer.attachTexture(attachment.mHandle, textureManager.resolve(attachment.mHandle), attachment.mTarget, attachment.mMip);
				}
				if (framebuffer.mResource.mFramebuffer.mColorAttachments) {
					framebuffer.mResource.mFramebuffer.initDrawBuffers();
				}
				if (debugName.has_value()) {
					NEO_LOG_V("Loaded framebuffer %s", debugName.value().c_str());
					framebuffer.mDebugName = debugName;
				}

				return framebuffer;
			}
		};
	}

	FramebufferManager::FramebufferManager() {
	}

	void FramebufferManager::_initImpl() {
		// Fallback is the backbuffer, which a default-constructed Framebuffer already points at.
		// Goodluck
		mFallback = std::make_shared<CachedResource<ManagedFramebuffer>>();
	}

	FramebufferManager::~FramebufferManager() {
		mFallback.reset();
	}

	[[nodiscard]] FramebufferHandle FramebufferManager::asyncLoad(HashedString id, FramebufferLoadDetails framebufferDetails, const TextureManager& textureManager) const {
		FramebufferHandle dstHandle = swizzleSrcId(id, framebufferDetails, textureManager);
		if (!isDiscardQueued(dstHandle) && (isValid(dstHandle) || isQueued(dstHandle))) {
			return dstHandle;
		}

		FramebufferAttachments attachments;
		util::visit(framebufferDetails,
			[&](FramebufferBuilder& builder) {
				for (int i = 0; i < builder.mAttachments.size(); i++) {
					auto& attachment = builder.mAttachments[i];
					attachments.mAttachments.emplace_back(FramebufferAttachment{
						textureManager.asyncLoad(
							swizzleTextureId(dstHandle.mHandle + i, attachment.mFormat, attachment.mTarget, attachment.mMip, builder.mSize),
							TextureBuilder{ attachment.mFormat, glm::u16vec3(builder.mSize, 0.0)}, std::string(id.data()) + "_" + std::to_string(i)),
						attachment.mTarget,
						attachment.mMip
					});
				}
			},
			[&](FramebufferExternalAttachments& externalAttachments) {
				attachments.mAttachments = externalAttachments;
				attachments.mExternallyOwned = true;
			},
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
		);

		return ResourceManagerInterface::asyncLoad(dstHandle, std::move(attachments), std::string(id.data()));
	}

	[[nodiscard]] FramebufferHandle FramebufferManager::_asyncLoadImpl(FramebufferHandle handle, FramebufferAttachments attachments, const std::optional<std::string>& debugName) const {
		{
			std::lock_guard<std::mutex> lock(mLoadQueueMutex);
			mLoadQueue.emplace_back(ResourceLoadDetails_Internal{ handle, std::move(attachments), debugName });
		}
		return handle;
	}

	Framebuffer& FramebufferManager::_resolveFramebuffer(FramebufferHandle handle) const {
		if (handle == 0) {
			return mFallback->mResource.mFramebuffer; // Special-case backbuffer
		}
		if (!ResourceManagerInterface::isValid(handle)) {
			NEO_LOG_W("Invalid resource requested! Did you check for validity?");
			mFallback->mResource.mFramebuffer.bind();
			return mFallback->mResource.mFramebuffer;
		}
		// Goes through the base resolve so that eviction sees the use.
		return const_cast<Framebuffer&>(ResourceManagerInterface::resolve(handle).mFramebuffer);
	}

	void FramebufferManager::_tickImpl() {
		TRACY_ZONE();
		NEO_ASSERT(mTextureManager != nullptr, "FramebufferManager was never given a TextureManager");

		{
			std::vector<ResourceLoadDetails_Internal> swapQueue = {};
			{
				std::lock_guard<std::mutex> lock(mLoadQueueMutex);
				std::swap(mLoadQueue, swapQueue);
				mLoadQueue.clear();
			}
			TRACY_GPUN("Load");
			for (auto& details : swapQueue) {
				TRACY_GPUN("Create Single");
				if (std::optional<CachedResource<ManagedFramebuffer>> framebuffer = FramebufferLoader{}.load(details.mLoadDetails, details.mDebugName, *mTextureManager)) {
					mCache.insert(details.mHandle, std::move(*framebuffer));
				}
				_finishPending(details.mHandle);
			}
		}

		{
			std::vector<FramebufferHandle> swapQueue;
			{
				std::lock_guard<std::mutex> lock(mDiscardQueueMutex);
				std::swap(mDiscardQueue, swapQueue);
				mDiscardQueue.clear();
			}

			if (!swapQueue.empty()) {
				TRACY_GPUN("Destroy");
				for (auto& handle : swapQueue) {
					TRACY_GPUN("Destroy Single");
					if (ResourceManagerInterface::isValid(handle)) {
						retire(handle);
					}
				}
			}
		}

		NEO_ASSERT(mTransactionQueue.empty(), "Framebuffer transactions unsupported");
	}

	void FramebufferManager::_destroyImpl(CachedResource<ManagedFramebuffer>& framebuffer) {
		NEO_ASSERT(mTextureManager != nullptr, "FramebufferManager was never given a TextureManager");
		ManagedFramebuffer& managed = framebuffer.mResource;
		if (!managed.mExternallyOwned) {
			for (auto& textureHandle : managed.mFramebuffer.mTextures) {
				mTextureManager->discard(textureHandle);
			}
		}
		managed.mFramebuffer.destroy();
	}

	void FramebufferManager::imguiEditor(std::function<void(const TextureHandle&)> textureFunc, TextureManager& textureManager) {
		if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableSetupColumn("Name/Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending);
			ImGui::TableSetupColumn("Attachments");
			ImGui::TableHeadersRow();

			mCache.forEach([&](const CachedResource<ManagedFramebuffer>& fb) {
				const HashedString::hash_type handle = fb.mHandle.mHandle;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (fb.mDebugName.has_value()) {
					ImGui::Text(fb.mResource.mExternallyOwned ? "%s" : "*%s", fb.mDebugName->c_str());
				}
				else {
					ImGui::Text(fb.mResource.mExternallyOwned ? "%d" : "*%d", handle);
				}
				if (textureManager.isValid(fb.mResource.mFramebuffer.mTextures[0])) {
					auto& firstTex = textureManager.resolve(fb.mResource.mFramebuffer.mTextures[0]);
					ImGui::Text("[%d, %d]", firstTex.mWidth, firstTex.mHeight);
				}
				ImGui::TableSetColumnIndex(1);
				for (auto texId = fb.mResource.mFramebuffer.mTextures.begin(); texId < fb.mResource.mFramebuffer.mTextures.end(); texId++) {
					if (textureManager.isValid(*texId)) {
						textureFunc(*texId);
						if (texId != std::prev(fb.mResource.mFramebuffer.mTextures.end())) {
							ImGui::SameLine();
						}
					}
				}
			});
			ImGui::EndTable();
		}
	}
}
