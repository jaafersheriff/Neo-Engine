#pragma once

#include "Util/Util.hpp"

#include "ResourceManager/TextureResourceManager.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include <entt/resource/cache.hpp>
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
		glm::uvec2 mSize;
		std::vector<TextureFormat> mFormats;

		FramebufferBuilder& setSize(glm::uvec2 size) {
			mSize = size;
			return *this;
		}

		FramebufferBuilder& attach(TextureFormat format) {
			mFormats.emplace_back(format);
			return *this;
		}

		bool operator==(const FramebufferBuilder& other) const {
			// Kinda faulty, but meh
			return other.mSize == mSize && other.mFormats == mFormats;
		}
	};

	struct FramebufferExternal {
		std::vector<TextureHandle> mTextureHandles;
	};
	using FramebufferLoadDetails = std::variant<FramebufferBuilder, FramebufferExternal>;

	struct PooledFramebuffer {
		Framebuffer mFramebuffer;
		uint8_t mFrameCount = 0;
		bool mUsedThisFrame = false;
		bool mExternallyOwned = false;
	};
	using FramebufferHandle = ResourceHandle<PooledFramebuffer>;
	struct FramebufferQueueItem {
		FramebufferHandle mHandle;
		std::vector<TextureHandle> mTexIDs;
		bool mExternallyOwned = false;
		std::optional<std::string> mDebugName;
	};

	class FramebufferResourceManager {
		friend ResourceManagers;
	public:

		FramebufferResourceManager::FramebufferResourceManager();
		FramebufferResourceManager::~FramebufferResourceManager();

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

		[[nodiscard]] FramebufferHandle asyncLoad(HashedString id, FramebufferLoadDetails details, const TextureResourceManager& textureManager) const;

	protected:

		void clear(const TextureResourceManager& textureManager);
		void tick(const TextureResourceManager& textureManager);

		void imguiEditor(std::function<void(Texture&)> textureFunc, TextureResourceManager& textureManager);

		mutable std::vector<FramebufferQueueItem> mQueue;
		entt::resource_cache<BackedResource<PooledFramebuffer>> mCache;
		std::shared_ptr<Framebuffer> mFallback;

	private:
		Framebuffer& _resolveFinal(FramebufferHandle id) const;
	};
}
