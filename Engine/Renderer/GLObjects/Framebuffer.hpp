#pragma once

#include "Texture2D.hpp"
#include "GLHelper.hpp"

#include "Util/Util.hpp"

#include <GL/glew.h>

#include <tracy/Tracy.hpp>

namespace neo {

    class Framebuffer {
    
    public: 
        GLuint mFBOID;
        int mColorAttachments = 0;
        std::vector<Texture *> mTextures;

        Framebuffer() {
            glGenFramebuffers(1, &mFBOID);
        }
        
        void bind() {
            ZoneScoped;
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
        
        void attachColorTexture(glm::uvec2 size, TextureFormat format) {
            Texture *t = new Texture2D(format, size, nullptr);
            _attachTexture(GL_COLOR_ATTACHMENT0 + mColorAttachments++, *t);
        }
        
        void attachDepthTexture(glm::ivec2 size, GLint filter, GLenum mode) {
            Texture *t = new Texture2D(TextureFormat{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, filter, mode }, size, nullptr);
            _attachTexture(GL_DEPTH_ATTACHMENT, *t);
        }
        
        void attachDepthTexture(Texture *t) {
            NEO_ASSERT(t->mFormat.mInternalFormat == GL_DEPTH_COMPONENT, "Invalid depth target format");
            NEO_ASSERT(t->mFormat.mBaseFormat == GL_DEPTH_COMPONENT, "Invalid depth target format"); // or depth_16, 24, 32
            _attachTexture(GL_DEPTH_ATTACHMENT, *t);
        }


        void attachStencilTexture(glm::uvec2 size, GLint filter, GLenum mode) {
            Texture* t = new Texture2D({ GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, filter, mode, GL_UNSIGNED_INT_24_8 }, size, nullptr);
            _attachTexture(GL_DEPTH_STENCIL_ATTACHMENT, *t);
        }
        
        void initDrawBuffers() {
            if (!mColorAttachments) {
                return;
            }
        
            bind();
            std::vector<GLenum> attachments;
            for (int i = 0; i < mColorAttachments; i++) {
                attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
            glDrawBuffers(mColorAttachments, attachments.data());
        }
        
        void resize(const glm::uvec2 size) {
            bind();
            glViewport(0, 0, size.x, size.y);
            for (auto& texture : mTextures) {
                texture->resize(size);
            }
        }
        
        void destroy() {
            for (auto texture : mTextures) {
                texture->destroy();
            }
            glDeleteFramebuffers(1, &mFBOID);
        }

        private:
            void _attachTexture(GLuint component, Texture &texture) {
                mTextures.emplace_back(&texture);
                bind();
                glFramebufferTexture2D(GL_FRAMEBUFFER, component, GL_TEXTURE_2D, texture.mTextureID, 0);
                CHECK_GL_FRAMEBUFFER();
           }
    };
}
