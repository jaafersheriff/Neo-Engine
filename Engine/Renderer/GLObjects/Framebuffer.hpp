#pragma once

#include "Renderer/Types.hpp"

#include "ResourceManager/TextureManager.hpp"

#include <vector>
#include <optional>

namespace neo {

	class Framebuffer {
	public:

		uint32_t mFBOID = 0;
		int mColorAttachments = 0;
		std::vector<TextureHandle> mTextures;

		void bind() const;
		void clear(glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags) const;

		void init(const std::optional<std::string>& debugName);
		void disableDraw() const;
		void disableRead() const;

		void attachTexture(TextureHandle id, const Texture& texture, const types::framebuffer::AttachmentTarget& target, uint8_t mip);
		void initDrawBuffers();
		void destroy() const; // TODO - why is this const :(
	};
}
