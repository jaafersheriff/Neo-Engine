#pragma once

#include "Texture.hpp"
#include "GLHelper.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

#include "Renderer/Types.hpp"

#include "ResourceManager/TextureResourceManager.hpp"

#include <vector>

namespace neo {

	class Framebuffer {
	public:

		uint32_t mFBOID = 0;
		int mColorAttachments = 0;
		std::vector<TextureHandle> mTextures;

		void bind() const;
		void clear(glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags) const;

		void init();
		void disableDraw() const;
		void disableRead() const;

		void attachTexture(TextureHandle id, const Texture& texture);
		void initDrawBuffers();
		void destroy();
	};
}
