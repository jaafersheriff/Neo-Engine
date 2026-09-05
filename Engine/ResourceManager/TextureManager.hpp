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
		void imguiEditor(std::function<void(const TextureHandle&)> textureFunc);

		// The CPU-side shape of a texture, by value. Anything that needs a texture's dimensions or
		// format wants this rather than resolve(): it copies out from under the cache's shared lock,
		// so the caller never holds a pointer into the cache and never sees the GL object at all.
		// nullopt for a handle the cache does not have, which folds what used to be a separate
		// isValid() call into the same lock acquisition.
		[[nodiscard]] std::optional<TextureDescriptor> getDescriptor(const TextureHandle& handle) const {
			if (const CachedResource<Texture>* entry = mCache.resolve(handle)) {
				return entry->mResource.getDescriptor();
			}
			return std::nullopt;
		}

	protected:
		[[nodiscard]] TextureHandle _asyncLoadImpl(TextureHandle id, TextureLoadDetails textureDetails, const std::optional<std::string>& debugName) const;
		void _destroyImpl(CachedResource<Texture>& texture);
		void _initImpl();
		void _tickImpl();
	private:
		void _insertLoaded(const ResourceLoadDetails_Internal& loadDetails, std::optional<CachedResource<Texture>>&& texture);
	};
}