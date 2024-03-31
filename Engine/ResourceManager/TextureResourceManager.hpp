#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "Util/Util.hpp"

#include <string>
#include <variant>

namespace neo {
	class ResourceManagers;

	// TODO - move to its own file
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

	// TODO - rename to something texture-specific 
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
		void imguiEditor(std::function<void(const Texture&)> textureFunc);

	protected:
		[[nodiscard]] TextureHandle _asyncLoadImpl(TextureHandle id, TextureLoadDetails textureDetails, std::string debugName) const;
		void _clearImpl();
		void _tickImpl();

	};
}