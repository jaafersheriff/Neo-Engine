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

            void attachColorTexture(Texture &texture) {
                attachTexture(GL_COLOR_ATTACHMENT0 + colorAttachments++, texture);
            }

            void initDrawBuffers() {
                if (!colorAttachments) {
                    return;
                }

                bind();
                std::vector<unsigned int> attachments;
                for (int i = 0; i < colorAttachments; i++) {
                    attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
                }
                CHECK_GL(glDrawBuffers(colorAttachments, attachments.data()));
            }

            void attachDepthTexture(Texture &texture) {
                attachTexture(GL_DEPTH_ATTACHMENT, texture);
            }

        private:
            void attachTexture(GLuint component, Texture &texture) {
                textures.emplace_back(&texture);
                bind();
                CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, component, GL_TEXTURE_2D, texture.textureId, 0));
                CHECK_GL_FRAMEBUFFER();
           }
    };
}