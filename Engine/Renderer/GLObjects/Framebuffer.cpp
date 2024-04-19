#include "Framebuffer.hpp"

#include "GLHelper.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

#ifdef DEBUG_MODE
#define CHECK_GL_FRAMEBUFFER() do {GLHelper::checkFrameBuffer(); } while(0)
#else
#define CHECK_GL_FRAMEBUFFER()
#endif

namespace neo {
	namespace {
		GLbitfield _getGLClearFlags(types::framebuffer::AttachmentBits flagBits) {

			GLbitfield flags = 0;
			if (flagBits.mClearBits & static_cast<uint8_t>(types::framebuffer::AttachmentBit::Color)) {
				flags |= GL_COLOR_BUFFER_BIT;
			}
			if (flagBits.mClearBits & static_cast<uint8_t>(types::framebuffer::AttachmentBit::Depth)) {
				flags |= GL_DEPTH_BUFFER_BIT;
			}
			if (flagBits.mClearBits & static_cast<uint8_t>(types::framebuffer::AttachmentBit::Stencil)) {
				flags |= GL_STENCIL_BUFFER_BIT;
			}

			return flags;
		}

		GLenum _getGLAttachment(types::framebuffer::AttachmentBit bit) {
			if (bit == types::framebuffer::AttachmentBit::Depth) {
				return GL_DEPTH_ATTACHMENT;
			}
			if (bit == types::framebuffer::AttachmentBit::Stencil) {
				return GL_DEPTH_STENCIL;
			}

			return GL_COLOR_ATTACHMENT0;
		}

	}

	void Framebuffer::init(std::optional<std::string> debugName) {
		glGenFramebuffers(1, &mFBOID);
		if (debugName.has_value() && !debugName.value().empty()) {
			bind();
			glObjectLabel(GL_FRAMEBUFFER, mFBOID, -1, debugName.value().c_str());
		}
		disableRead();
	}

	void Framebuffer::bind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, mFBOID);
	}

	void Framebuffer::disableDraw() const {
		bind();
		glDrawBuffer(GL_NONE);
	}

	void Framebuffer::disableRead() const {
		bind();
		glReadBuffer(GL_NONE);
	}

	void Framebuffer::attachTexture(TextureHandle id, const Texture& texture) {
		NEO_ASSERT(texture.mFormat.mTarget == types::texture::Target::Texture2D, "Framebuffers need 2D textures");

		types::framebuffer::AttachmentBit attachment;
		switch (TextureFormat::deriveBaseFormat(texture.mFormat.mInternalFormat)) {
			case types::texture::BaseFormats::Depth:
				attachment = types::framebuffer::AttachmentBit::Depth;
				break;
			case types::texture::BaseFormats::DepthStencil:
				attachment = types::framebuffer::AttachmentBit::Stencil;
				break;
			default:
				attachment = types::framebuffer::AttachmentBit::Color;
				mColorAttachments++;
				break;
		}

		mTextures.emplace_back(id);
		bind();
		glFramebufferTexture(GL_FRAMEBUFFER, _getGLAttachment(attachment), texture.mTextureID, 0);
		CHECK_GL_FRAMEBUFFER();
	}

	void Framebuffer::initDrawBuffers() {
		NEO_ASSERT(mColorAttachments, "Attempting to init FBO without any color attachments");
		NEO_ASSERT(mColorAttachments < GL_MAX_COLOR_ATTACHMENTS, "You attached too many textures to this framebuffer");

		bind();
		std::vector<GLenum> attachments;
		for (int i = 0; i < mColorAttachments; i++) {
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		}
		glDrawBuffers(mColorAttachments, attachments.data());
		CHECK_GL_FRAMEBUFFER();
	}

	void Framebuffer::clear(glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags) const {
		NEO_ASSERT(mTextures.size(), "Attempting to clear framebuffer with no textures");
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(_getGLClearFlags(clearFlags));
	}

	void Framebuffer::destroy() {
		glDeleteFramebuffers(1, &mFBOID);
		mColorAttachments = 0;
		mFBOID = 0;
	}

}