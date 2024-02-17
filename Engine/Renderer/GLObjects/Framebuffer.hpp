#pragma once

#include "Texture.hpp"
#include "GLHelper.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>
#include <vector>

namespace neo {

	class Framebuffer {
	
	public: 
		GLuint mFBOID = 0;
		int mColorAttachments = 0;
		std::vector<Texture *> mTextures;

		void init() {
			glGenFramebuffers(1, &mFBOID);
		}
		
		void bind() {
			TRACY_ZONE();
			glBindFramebuffer(GL_FRAMEBUFFER, mFBOID);
		}
		
		void disableDraw() {
			bind();
			glDrawBuffer(GL_NONE);
		}
		
		void disableRead() {
			bind();
			glReadBuffer(GL_NONE);
		}
		
		void attachColorTexture(glm::uvec2 size, TextureFormat_DEPRECATED format) {
			NEO_ASSERT(format.mTarget == TextureTarget::Texture2D, "Framebuffers need 2D textures");
			Texture *t = new Texture(format, size, nullptr);
			_attachTexture(GL_COLOR_ATTACHMENT0 + mColorAttachments++, *t);
		}
		
		void attachDepthTexture(glm::ivec2 size, GLint format, GLint filter, GLenum mode) {
			NEO_ASSERT(format == GL_DEPTH_COMPONENT32F || format == GL_DEPTH_COMPONENT24 || format == GL_DEPTH_COMPONENT16, "Incompatible depth format");
			Texture *t = new Texture(TextureFormat_DEPRECATED{ TextureTarget::Texture2D, format, GL_DEPTH_COMPONENT, filter, mode }, size, nullptr);
			_attachTexture(GL_DEPTH_ATTACHMENT, *t);
		}
		
		void attachDepthTexture(Texture *t) {
			NEO_ASSERT(t->mFormat.mTarget == TextureTarget::Texture2D, "Framebuffers need 2D textures");
			NEO_ASSERT(t->mFormat.mInternalFormat == GL_DEPTH_COMPONENT, "Invalid depth target format");
			NEO_ASSERT(t->mFormat.mBaseFormat == GL_DEPTH_COMPONENT, "Invalid depth target format"); // or depth_16, 24, 32
			_attachTexture(GL_DEPTH_ATTACHMENT, *t);
		}


		void attachStencilTexture(glm::uvec2 size, GLint filter, GLenum mode) {
			Texture* t = new Texture({ TextureTarget::Texture2D, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, filter, mode, GL_UNSIGNED_INT_24_8 }, size, nullptr);
			_attachTexture(GL_DEPTH_STENCIL_ATTACHMENT, *t);
		}
		
		void initDrawBuffers() {
			NEO_ASSERT(mColorAttachments, "Attempting to init FBO without any color attachments");
		
			bind();
			std::vector<GLenum> attachments;
			for (int i = 0; i < mColorAttachments; i++) {
				attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
			}
			glDrawBuffers(mColorAttachments, attachments.data());
			CHECK_GL_FRAMEBUFFER();
		}
	   
		void clear(glm::vec4 clearColor, GLbitfield clearFlags) {
			NEO_ASSERT(mTextures.size(), "Attempting to clear framebuffer with no textures");
			bind();
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glClear(clearFlags);
		}
		
		void destroy() {
			for (auto texture : mTextures) {
				texture->destroy();
				delete texture;
			}
			mTextures.clear();
			glDeleteFramebuffers(1, &mFBOID);
			mColorAttachments = 0;
			mFBOID = 0;
		}

		private:
			void _attachTexture(GLuint component, Texture &texture) {
				mTextures.emplace_back(&texture);
				bind();
				glFramebufferTexture(GL_FRAMEBUFFER, component, texture.mTextureID, 0);
				CHECK_GL_FRAMEBUFFER();
		   }
	};
}
