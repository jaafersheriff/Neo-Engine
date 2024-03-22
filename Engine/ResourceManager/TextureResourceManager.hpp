#pragma once

#include "Renderer/GLObjects/Texture.hpp"

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>

#include <memory>
#include <string>

namespace {
	struct FileLoadDetails;
}
namespace neo {
	class ResourceManagers;

	struct STBImageData {
		STBImageData(const char* _filePath, types::texture::BaseFormats baseFormat, bool flip);
		~STBImageData();

		operator bool() const {
			return mData != nullptr && mWidth > 0 && mHeight > 0;
		}

		std::string mFilePath;
		uint8_t* mData = nullptr;
		int mWidth = 0;
		int mHeight = 0;
	};


	using TextureHandle = entt::id_type;
	class TextureResourceManager {
		friend ResourceManagers;
	public:

		struct TextureBuilder {
			TextureFormat mFormat;
			glm::u16vec3 mDimensions = glm::u16vec3(0);
			const uint8_t* mData = nullptr;
		};
		struct FileLoadDetails {
			std::vector<std::string> mFilePaths;
			TextureFormat mFormat;
		};


		TextureResourceManager();
		~TextureResourceManager();
		bool isValid(TextureHandle id) const;
		Texture& get(HashedString id);
		const Texture& get(HashedString id) const;
		Texture& get(TextureHandle id);
		const Texture& get(TextureHandle id) const;

		[[nodiscard]] TextureHandle asyncLoad(const char* filePath, TextureFormat format) const;
		[[nodiscard]] TextureHandle asyncLoad(const char* name, std::vector<std::string> filePath, TextureFormat format) const;
		[[nodiscard]] TextureHandle asyncLoad(HashedString id, TextureBuilder& textureDetails) const;

		void clear();
	private:

		void _tick();
		mutable std::map<TextureHandle, TextureBuilder> mQueue;

		mutable std::map<TextureHandle, FileLoadDetails> mFileLoadQueue;
		using TextureCache = entt::resource_cache<Texture>;
		TextureCache mTextureCache;
		std::shared_ptr<Texture> mDummyTexture;
	};
}