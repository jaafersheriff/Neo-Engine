#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLHelper.hpp"

#include "Texture.hpp"

#include <vector>

namespace neo {

    class Framebuffer {
    
       public: 
            GLuint fboId;
            int colorAttachments = 0;
            std::vector<Texture *> textures;

            void generate() {
                CHECK_GL(glGenFramebuffers(1, &fboId));
            }

            void bind() {
                CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fboId));
            }

            void disableDraw() {
                bind();
                CHECK_GL(glDrawBuffer(GL_NONE));
            }

            void disableRead() {
                bind();
                CHECK_GL(glReadBuffer(GL_NONE));
            }

            void attachColorTexture(glm::ivec2 size, int comp, GLint inFormat, GLenum format, GLint filter, GLenum mode) {
                Texture *t = new Texture2D;
                t->mWidth = size.x;
                t->mHeight = size.y;
                t->mComponents = comp;
                t->upload(inFormat, format, filter, mode);
                attachTexture(GL_COLOR_ATTACHMENT0 + colorAttachments++, *t);
            }

            void attachDepthTexture(glm::ivec2 size, GLint filter, GLenum mode) {
                Texture *t = new Texture2D;
                t->mWidth = size.x;
                t->mHeight = size.y;
                t->mComponents = 1;
                t->upload(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, filter, mode);
                attachTexture(GL_DEPTH_ATTACHMENT, *t);
            }

            void initDrawBuffers() {
                if (!colorAttachments) {
                    return;
                }

                bind();
                std::vector<GLenum> attachments;
                for (int i = 0; i < colorAttachments; i++) {
                    attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
                }
                CHECK_GL(glDrawBuffers(colorAttachments, attachments.data()));
            }

            void resize(const glm::uvec2 size) {
                bind();
                CHECK_GL(glViewport(0, 0, size.x, size.y));
                for (auto& texture : textures) {
                    texture->resize(size);
                }
            }

        private:
            void attachTexture(GLuint component, Texture &texture) {
                textures.emplace_back(&texture);
                bind();
                CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, component, GL_TEXTURE_2D, texture.mTextureID, 0));
                CHECK_GL_FRAMEBUFFER();
           }
    };
}