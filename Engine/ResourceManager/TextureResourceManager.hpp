#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "Loader/STBIImageData.hpp"

#include "Util/Util.hpp"

#include <string>
#include <variant>

namespace neo {
	class ResourceManagers;

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
	using TextureLoadDetails = std::variant<TextureBuilder, FileLoadDetails>;
	using TextureHandle = ResourceHandle<Texture>;

	class TextureResourceManager final : public ResourceManagerInterface<TextureResourceManager, Texture, TextureLoadDetails> {
		friend ResourceManagerInterface;
	public:

		TextureResourceManager();
		~TextureResourceManager();
		void imguiEditor(std::function<void(const Texture&)> textureFunc);

	protected:
		[[nodiscard]] TextureHandle _asyncLoadImpl(TextureHandle id, TextureLoadDetails textureDetails, std::optional<std::string> debugName) const;
		void _clearImpl();
		void _tickImpl();
	};
}