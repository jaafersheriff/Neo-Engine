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
		struct Attachment {
			TextureHandle mTextureHandle = NEO_INVALID_HANDLE;
			types::framebuffer::AttachmentTarget mTarget = types::framebuffer::AttachmentTarget::Target2D;
			int mMip = 0;
		};
		std::vector<Attachment> mAttachments;

		void bind() const;
		void clear(glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags) const;

		void init(const std::optional<std::string>& debugName);
		void disableDraw() const;
		void disableRead() const;

		void attachTexture(TextureHandle textureHandle, const Texture& texture, const types::framebuffer::AttachmentTarget& target, uint8_t mip);
		void initDrawBuffers();
		void destroy();
	};
}
