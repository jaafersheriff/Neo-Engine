#pragma once

#include "Util/Util.hpp"

#include "ResourceManager/TextureManager.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include <ext/entt_incl.hpp>
#include <variant>
#include <optional>
#include <memory>

// Framebuffers are weird because
//   - They're pooled
//   - They require textures (handles and backed resources) from another resource manager 
// Keep the same interface, but it does it's own things internally
namespace neo {
	class ResourceManagers;
	struct TextureFormat;

	struct FramebufferBuilder {
		// Hmmm this forces all internally-created textures to be the same size...
		glm::uvec2 mSize;
		struct BuilderAttachment {
			TextureFormat mFormat;
			types::framebuffer::AttachmentTarget mTarget;
			uint8_t mMip;
			bool operator==(const BuilderAttachment& other) const {
				return mFormat == other.mFormat && mTarget == other.mTarget && mMip == other.mMip;
			}

		};
		std::vector<BuilderAttachment> mAttachments;

		FramebufferBuilder& setSize(glm::uvec2 size) {
			mSize = size;
			return *this;
		}

		FramebufferBuilder& attach(TextureFormat format, types::framebuffer::AttachmentTarget target = types::framebuffer::AttachmentTarget::Target2D, uint8_t mip = 0) {
			mAttachments.emplace_back(BuilderAttachment{ format, target, mip });
			return *this;
		}

		bool operator==(const FramebufferBuilder& other) const {
			return other.mSize == mSize && other.mAttachments == mAttachments;
		}
	};

	struct FramebufferAttachment {
		TextureHandle mHandle = NEO_INVALID_HANDLE;
		types::framebuffer::AttachmentTarget mTarget = types::framebuffer::AttachmentTarget::Target2D;
		uint8_t mMip = 0;
	};

	using FramebufferExternalAttachments = std::vector<FramebufferAttachment>;
	using FramebufferLoadDetails = std::variant<FramebufferBuilder, FramebufferExternalAttachments>;

	struct PooledFramebuffer {
		Framebuffer mFramebuffer;
		uint8_t mFrameCount = 0;
		bool mUsedThisFrame = false;
		bool mExternallyOwned = false;
	};
	using FramebufferHandle = ResourceHandle<PooledFramebuffer>;
	struct FramebufferQueueItem {
		FramebufferHandle mHandle;
		std::vector<FramebufferAttachment> mAttachments;
		bool mExternallyOwned = false;
		std::optional<std::string> mDebugName;
	};

	class FramebufferManager {
		friend ResourceManagers;
	public:

		FramebufferManager::FramebufferManager();
		FramebufferManager::~FramebufferManager();

		bool isValid(FramebufferHandle id) const {
			return mCache.contains(id.mHandle);
		}

		bool isQueued(FramebufferHandle id) const {
			// TODO - this is a linear search :(
			// But maybe it's fine because we shouldn't be queueing up a bunch of stuff every single frame..
			for (auto& res : mQueue) {
				if (id == res.mHandle) {
					return true;
				}
			}
			return false;
		}

		const Framebuffer& resolve(FramebufferHandle id) const {
			return _resolveFinal(id);
		}

		Framebuffer& resolve(FramebufferHandle id) {
			return _resolveFinal(id);
		}

		[[nodiscard]] FramebufferHandle asyncLoad(HashedString id, FramebufferLoadDetails details, const TextureManager& textureManager) const;

	protected:

		void clear(const TextureManager& textureManager);
		void tick(const TextureManager& textureManager);

		void imguiEditor(std::function<void(const TextureHandle&)> textureFunc, TextureManager& textureManager);

		mutable std::vector<FramebufferQueueItem> mQueue;
		std::shared_ptr<Framebuffer> mFallback;

	private:
		Framebuffer& _resolveFinal(FramebufferHandle id) const;

		struct FramebufferLoader {
			using result_type = std::shared_ptr<BackedResource<PooledFramebuffer>>;
			result_type operator()(const FramebufferQueueItem& details, const TextureManager& textureManager) const;
		};

		entt::resource_cache<BackedResource<PooledFramebuffer>, FramebufferLoader> mCache;
	};
}
