#include "Framebuffer.hpp"

namespace neo {
	namespace {
		void _attachTexture(Framebuffer& fb, GLenum component, Texture& texture) {
			fb.mTextures.emplace_back(&texture);
			fb.bind();
			glFramebufferTexture(GL_FRAMEBUFFER, component, texture.mTextureID, 0);
			CHECK_GL_FRAMEBUFFER();
		}
	}

	void Framebuffer::init() {
		glGenFramebuffers(1, &mFBOID);
	}

	void Framebuffer::bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, mFBOID);
	}

	void Framebuffer::disableDraw() {
		bind();
		glDrawBuffer(GL_NONE);
	}

	void Framebuffer::disableRead() {
		bind();
		glReadBuffer(GL_NONE);
	}

	void Framebuffer::attachColorTexture(glm::uvec2 size, TextureFormat format) {
		NEO_ASSERT(format.mTarget == types::texture::Target::Texture2D, "Framebuffers need 2D textures");
		Texture* t = new Texture(format, size, nullptr);
		_attachTexture(*this, GL_COLOR_ATTACHMENT0 + mColorAttachments++, *t);
	}

	void Framebuffer::attachDepthTexture(glm::ivec2 size, types::texture::InternalFormats format, TextureFilter filter, TextureWrap wrap) {
		Texture* t = new Texture(TextureFormat{
			types::texture::Target::Texture2D,
			format,
			types::texture::BaseFormats::Depth,
			filter, 
			wrap }, 
		size, nullptr);
		_attachTexture(*this, GL_DEPTH_ATTACHMENT, *t);
	}

	void Framebuffer::attachDepthTexture(Texture* t) {
		NEO_ASSERT(t->mFormat.mTarget == types::texture::Target::Texture2D, "Framebuffers need 2D textures");
		NEO_ASSERT(t->mFormat.mInternalFormat == types::texture::InternalFormats::Depth16 
			|| t->mFormat.mInternalFormat == types::texture::InternalFormats::Depth24 
			|| t->mFormat.mInternalFormat == types::texture::InternalFormats::Depth32 
			, "Invalid depth target format");
		NEO_ASSERT(t->mFormat.mBaseFormat == types::texture::BaseFormats::Depth, "Invalid depth target format");
		_attachTexture(*this, GL_DEPTH_ATTACHMENT, *t);
	}

	void Framebuffer::attachStencilTexture(glm::uvec2 size, TextureFilter filter, TextureWrap wrap) {
		Texture* t = new Texture({ 
			types::texture::Target::Texture2D, 
			types::texture::InternalFormats::Depth24Stencil8, 
			types::texture::BaseFormats::DepthStencil,
			filter, 
			wrap, 
			types::ByteFormats::UnsignedInt24_8 }, 
		size, nullptr);
		_attachTexture(*this, GL_DEPTH_STENCIL_ATTACHMENT, *t);
	}

	void Framebuffer::initDrawBuffers() {
		NEO_ASSERT(mColorAttachments, "Attempting to init FBO without any color attachments");

		bind();
		std::vector<GLenum> attachments;
		for (int i = 0; i < mColorAttachments; i++) {
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		}
		glDrawBuffers(mColorAttachments, attachments.data());
		CHECK_GL_FRAMEBUFFER();
	}

	void Framebuffer::clear(glm::vec4 clearColor, GLbitfield clearFlags) {
		NEO_ASSERT(mTextures.size(), "Attempting to clear framebuffer with no textures");
		bind();
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(clearFlags);
	}

	void Framebuffer::destroy() {
		for (auto texture : mTextures) {
			texture->destroy();
			delete texture;
		}
		mTextures.clear();
		glDeleteFramebuffers(1, &mFBOID);
		mColorAttachments = 0;
		mFBOID = 0;
	}

}