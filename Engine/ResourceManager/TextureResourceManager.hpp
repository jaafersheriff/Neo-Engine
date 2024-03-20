#pragma once

#include "Renderer/GLObjects/Texture.hpp"

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>

#include <memory>
#include <string>

namespace neo {
	class ResourceManagers;

	using TextureHandle = entt::id_type;
	class TextureResourceManager {
		friend ResourceManagers;
	public:
		struct TextureBuilder {
			TextureFormat mFormat;
			glm::u16vec3 mDimensions = glm::u16vec3(0);
			const uint8_t* data = nullptr;
		};

		TextureResourceManager();
		~TextureResourceManager();
		bool isValid(TextureHandle id) const;
		Texture& get(HashedString id);
		const Texture& get(HashedString id) const;
		Texture& get(TextureHandle id);
		const Texture& get(TextureHandle id) const;

		[[nodiscard]] TextureHandle asyncLoad(const char* filePath, TextureFormat format) const;
		[[nodiscard]] TextureHandle asyncLoad(HashedString id, TextureBuilder& textureDetails) const;

		void clear();
	private:
		void _tick();
		mutable std::vector<std::pair<TextureHandle, TextureBuilder>> mQueue;
		mutable std::vector<std::pair<std::string, TextureFormat>> mFileLoadQueue;
		using TextureCache = entt::resource_cache<Texture>;
		TextureCache mTextureCache;
		std::shared_ptr<Texture> mDummyTexture;
	};
}