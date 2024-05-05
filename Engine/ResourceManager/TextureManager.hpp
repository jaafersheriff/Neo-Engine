#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "Util/Util.hpp"

#include <string>
#include <variant>

namespace neo {
	class ResourceManagers;

	struct TextureBuilder {
		TextureFormat mFormat;
		glm::u16vec3 mDimensions = glm::u16vec3(0);
		uint8_t* mData = nullptr;

		TextureBuilder& setFormat(TextureFormat format) {
			mFormat = format;
			return *this;
		}
		TextureBuilder& setDimension(glm::u16vec3 dim) {
			mDimensions = dim;
			return *this;
		}
		TextureBuilder& setData(uint8_t* data) {
			mData = data;
			return *this;
		}
	};

	struct TextureFiles {
		std::vector<std::string> mFilePaths;
		TextureFormat mFormat;
	};
	using TextureLoadDetails = std::variant<TextureBuilder, TextureFiles>;
	using TextureHandle = ResourceHandle<Texture>;

	class TextureManager final : public ResourceManagerInterface<TextureManager, Texture, TextureLoadDetails> {
		friend ResourceManagerInterface;
	public:

		TextureManager();
		~TextureManager();
		void imguiEditor(std::function<void(const Texture&)> textureFunc);

	protected:
		[[nodiscard]] TextureHandle _asyncLoadImpl(TextureHandle id, TextureLoadDetails textureDetails, std::optional<std::string> debugName) const;
		void _destroyImpl(BackedResource<Texture>& texture);
		void _tickImpl();
	};
}