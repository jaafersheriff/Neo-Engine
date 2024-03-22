#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "Util/Util.hpp"

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

	struct TextureBuilder {
		TextureFormat mFormat;
		glm::u16vec3 mDimensions = glm::u16vec3(0);
		const uint8_t* mData = nullptr;
	};
	struct FileLoadDetails {
		std::vector<std::string> mFilePaths;
		TextureFormat mFormat;
	};

	using TextureHandle = entt::id_type;
	using TextureLoadDetails = std::variant<TextureBuilder, FileLoadDetails>;

	class TextureResourceManager final : public ResourceManagerInterface<TextureResourceManager, TextureHandle, Texture, TextureLoadDetails> {
		friend ResourceManagerInterface;
	public:

		TextureResourceManager();
		~TextureResourceManager();

	protected:
		[[nodiscard]] TextureHandle _asyncLoadImpl(HashedString id, TextureLoadDetails& textureDetails) const;
		void _clearImpl();
		void _tickImpl();
	};
}