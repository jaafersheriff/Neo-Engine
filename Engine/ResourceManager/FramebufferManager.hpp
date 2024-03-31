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

	struct PooledFramebufferDetails_New {
		glm::uvec2 mSize;
		std::vector<TextureFormat> mFormats;

		PooledFramebufferDetails_New& setSize(glm::uvec2 size) {
			mSize = size;
			return *this;
		}

		PooledFramebufferDetails_New& attach(TextureFormat format) {
			mFormats.emplace_back(format);
			return *this;
		}

		bool operator==(const PooledFramebufferDetails_New& other) const {
			// Kinda faulty, but meh
			return other.mSize == mSize && other.mFormats == mFormats;
		}
	};
	using FramebufferLoadDetails = std::variant<PooledFramebufferDetails_New, std::vector<TextureHandle>>;
	using QueuedDetails = std::pair<std::vector<TextureHandle>, bool>;

	struct PooledFramebuffer_New {
		Framebuffer mFramebuffer;
		uint8_t mFrameCount = 0;
		bool mUsedThisFrame = false;
		std::optional<std::string> mName = std::nullopt;
		bool mFullyOwned;
	};

	using FramebufferHandle = entt::id_type;
	class FramebufferResourceManager {
		friend ResourceManagers;
	public:

		FramebufferResourceManager::FramebufferResourceManager();
		FramebufferResourceManager::~FramebufferResourceManager();

		bool isValid(FramebufferHandle id) const {
			return mCache.contains(id);
		}

		bool isQueued(FramebufferHandle id) const {
			return mQueue.find(id) != mQueue.end();
		}

		Framebuffer& resolve(HashedString id) {
			return _resolveFinal(id);
		}

		const Framebuffer& resolve(HashedString id) const {
			return _resolveFinal(id);
		}

		const Framebuffer& resolve(FramebufferHandle id) const {
			return _resolveFinal(id);
		}

		Framebuffer& resolve(FramebufferHandle id) {
			return _resolveFinal(id);
		}

		[[nodiscard]] FramebufferHandle asyncLoad(const TextureResourceManager& textureManager, HashedString id, FramebufferLoadDetails details) const;

	protected:

		void clear(const TextureResourceManager& textureManager);
		void tick(const TextureResourceManager& textureManager);

		void imguiEditor();

		mutable std::map<FramebufferHandle, QueuedDetails> mQueue;
		entt::resource_cache<PooledFramebuffer_New> mCache;
		std::shared_ptr<Framebuffer> mFallback;

	private:
		Framebuffer& _resolveFinal(FramebufferHandle id) const;
	};
}
