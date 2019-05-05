#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLHelper.hpp"

#include "Texture.hpp"

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

            void attachColorTexture(glm::ivec2 size, int comp, TextureFormat format) {
                Texture *t = new Texture2D;
                t->mFormat = format;
                t->mWidth = size.x;
                t->mHeight = size.y;
                t->mComponents = comp;
                t->upload();
                attachTexture(GL_COLOR_ATTACHMENT0 + mColorAttachments++, *t);
            }

            void attachDepthTexture(glm::ivec2 size, GLint filter, GLenum mode) {
                Texture *t = new Texture2D;
                t->mFormat = TextureFormat{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, filter, mode };
                t->mWidth = size.x;
                t->mHeight = size.y;
                t->mComponents = 1;
                t->upload();
                attachTexture(GL_DEPTH_ATTACHMENT, *t);
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
            void attachTexture(GLuint component, Texture &texture) {
                mTextures.emplace_back(&texture);
                bind();
                CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, component, GL_TEXTURE_2D, texture.mTextureID, 0));
                CHECK_GL_FRAMEBUFFER();
           }
    };
}