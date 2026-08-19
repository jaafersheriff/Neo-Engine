#pragma once

#include "Util/Util.hpp"

#include "ResourceManager/ResourceManagerInterface.hpp"
#include "ResourceManager/TextureManager.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include <variant>
#include <optional>
#include <memory>

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
	// What callers ask for
	using FramebufferLoadDetails = std::variant<FramebufferBuilder, FramebufferExternalAttachments>;
	// What actually goes on the load queue
	struct FramebufferAttachments {
		std::vector<FramebufferAttachment> mAttachments;
		bool mExternallyOwned = false;
	};

	struct ManagedFramebuffer {
		Framebuffer mFramebuffer;
		bool mExternallyOwned = false;
	};
	using FramebufferHandle = ResourceHandle<ManagedFramebuffer>;

	class FramebufferManager final : public ResourceManagerInterface<FramebufferManager, ManagedFramebuffer, FramebufferAttachments, 5> {
		friend ResourceManagers;
		friend ResourceManagerInterface;
	public:

		FramebufferManager();
		~FramebufferManager();

		// Handle 0 is the backbuffer, which no manager owns - it is always valid and never queued.
		bool isValid(FramebufferHandle handle) const {
			return handle == 0 || ResourceManagerInterface::isValid(handle);
		}

		bool isQueued(FramebufferHandle handle) const {
			return handle != 0 && ResourceManagerInterface::isQueued(handle);
		}

		const Framebuffer& resolve(FramebufferHandle handle) const {
			return _resolveFramebuffer(handle);
		}

		Framebuffer& resolve(FramebufferHandle handle) {
			return _resolveFramebuffer(handle);
		}

		[[nodiscard]] FramebufferHandle asyncLoad(HashedString id, FramebufferLoadDetails details, const TextureManager& textureManager) const;

		void imguiEditor(std::function<void(const TextureHandle&)> textureFunc, TextureManager& textureManager);

	protected:
		[[nodiscard]] FramebufferHandle _asyncLoadImpl(FramebufferHandle handle, FramebufferAttachments attachments, const std::optional<std::string>& debugName) const;
		void _destroyImpl(CachedResource<ManagedFramebuffer>& framebuffer);
		void _initImpl();
		void _tickImpl();

	private:
		Framebuffer& _resolveFramebuffer(FramebufferHandle handle) const;

		// Required for cache eviction
		const TextureManager* mTextureManager = nullptr;
	};
}
