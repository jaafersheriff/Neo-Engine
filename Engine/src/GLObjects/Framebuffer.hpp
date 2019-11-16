#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLHelper.hpp"

#include "Texture2D.hpp"

#include <vector>

namespace neo {

    class Framebuffer {
    
       public: 
            GLuint mFBOID;
            int mColorAttachments = 0;
            std::vector<Texture *> mTextures;

            void generate() {
                CHECK_GL(glGenFramebuffers(1, &mFBOID));
            }

            void bind() {
                CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, mFBOID));
            }

            void disableDraw() {
                bind();
                CHECK_GL(glDrawBuffer(GL_NONE));
            }

            void disableRead() {
                bind();
                CHECK_GL(glReadBuffer(GL_NONE));
            }

            void attachColorTexture(glm::uvec2 size, TextureFormat format) {
                Texture *t = new Texture2D(format, size, nullptr);
                _attachTexture(GL_COLOR_ATTACHMENT0 + mColorAttachments++, *t);
            }

            void attachDepthTexture(glm::ivec2 size, GLint filter, GLenum mode) {
                Texture *t = new Texture2D(TextureFormat{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, filter, mode }, size, nullptr);
                _attachTexture(GL_DEPTH_ATTACHMENT, *t);
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
                CHECK_GL(glDrawBuffers(mColorAttachments, attachments.data()));
            }

            void resize(const glm::uvec2 size) {
                bind();
                CHECK_GL(glViewport(0, 0, size.x, size.y));
                for (auto& texture : mTextures) {
                    texture->resize(size);
                }
            }

            void destroy() {
                for (auto texture : mTextures) {
                    texture->destroy();
                }
                CHECK_GL(glDeleteFramebuffers(1, &mFBOID));
            }

        private:
            void _attachTexture(GLuint component, Texture &texture) {
                mTextures.emplace_back(&texture);
                bind();
                CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, component, GL_TEXTURE_2D, texture.mTextureID, 0));
                CHECK_GL_FRAMEBUFFER();
           }
    };
}
