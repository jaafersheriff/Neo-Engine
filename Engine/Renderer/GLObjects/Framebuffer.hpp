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
		// What was attached, and how. A framebuffer that attaches mip 2 of a texture, or one face of a
		// cubemap, used to forget which - so nothing downstream could preview the right slice.
		struct Attachment {
			TextureHandle mTextureHandle = NEO_INVALID_HANDLE;
			types::framebuffer::AttachmentTarget mTarget = types::framebuffer::AttachmentTarget::Target2D;
			uint8_t mMip = 0;
		};
		std::vector<Attachment> mAttachments;

		void bind() const;
		void clear(glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags) const;

		void init(const std::optional<std::string>& debugName);
		void disableDraw() const;
		void disableRead() const;

		void attachTexture(TextureHandle id, const Texture& texture, const types::framebuffer::AttachmentTarget& target, uint8_t mip);
		void initDrawBuffers();
		void destroy();
	};
}
