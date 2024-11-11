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

		GLenum _getGLAttachment(types::framebuffer::AttachmentBit bit, int colorAttachmentOffset) {
			if (bit == types::framebuffer::AttachmentBit::Depth) {
				return GL_DEPTH_ATTACHMENT;
			}
			if (bit == types::framebuffer::AttachmentBit::Stencil) {
				return GL_DEPTH_STENCIL;
			}
			return GL_COLOR_ATTACHMENT0 + colorAttachmentOffset;
		}

		GLenum _getGLTarget(types::framebuffer::AttachmentTarget target) {
			switch (target) {
			case types::framebuffer::AttachmentTarget::Target2D:
				return GL_TEXTURE_2D;
			case types::framebuffer::AttachmentTarget::TargetCubeX_Positive:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			case types::framebuffer::AttachmentTarget::TargetCubeX_Negative:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			case types::framebuffer::AttachmentTarget::TargetCubeY_Positive:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			case types::framebuffer::AttachmentTarget::TargetCubeY_Negative:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			case types::framebuffer::AttachmentTarget::TargetCubeZ_Positive:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			case types::framebuffer::AttachmentTarget::TargetCubeZ_Negative:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			default:
				return 0;
			}
		}

	}

	void Framebuffer::init(const std::optional<std::string>& debugName) {
		glGenFramebuffers(1, &mFBOID);
		if (debugName.has_value() && !debugName.value().empty()) {
			bind();
			glObjectLabel(GL_FRAMEBUFFER, mFBOID, -1, debugName.value().c_str());
		}
		glReadBuffer(GL_NONE);
	}

	void Framebuffer::bind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, mFBOID);
	}

	void Framebuffer::attachTexture(TextureHandle id, const Texture& texture, const types::framebuffer::AttachmentTarget& target, uint8_t mip) {
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, _getGLAttachment(attachment, mColorAttachments - 1), _getGLTarget(target), texture.mTextureID, mip);
		CHECK_GL_FRAMEBUFFER();
	}

	void Framebuffer::initDrawBuffers() {
		NEO_ASSERT(mColorAttachments < GL_MAX_COLOR_ATTACHMENTS, "You attached too many textures to this framebuffer");

		bind();
		if (mColorAttachments) {
			std::vector<GLenum> attachments;
			for (int i = 0; i < mColorAttachments; i++) {
				attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
			}
			glDrawBuffers(mColorAttachments, attachments.data());
		}
		else {
			// No color
			glDrawBuffer(GL_NONE);
		}
		CHECK_GL_FRAMEBUFFER();
	}

	void Framebuffer::clear(glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags) const {
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(_getGLClearFlags(clearFlags));
	}

	void Framebuffer::destroy() {
		glDeleteFramebuffers(1, &mFBOID);
		mColorAttachments = 0;
		mFBOID = 0;
	}

}