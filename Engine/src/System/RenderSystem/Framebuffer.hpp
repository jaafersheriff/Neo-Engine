#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "Util/GLHelper.hpp"

#include "Model/Texture.hpp"

namespace neo {

    class Framebuffer {
    
        // TODO - ability to add color render buffers
        public: 
            GLuint fboId;
            int colorAttachments = 0;

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

            void attachColorTexture(const Texture &texture) {
                attachTexture(GL_COLOR_ATTACHMENT0 + colorAttachments++, texture.textureId);
            }

            void attachDepthTexture(const Texture &texture) {
                attachTexture(GL_DEPTH_ATTACHMENT, texture.textureId);
            }

        private:
            void attachTexture(GLuint component, GLuint id) {
                bind();
                CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, component, GL_TEXTURE_2D, id, 0));
                CHECK_GL_FRAMEBUFFER();
           }
    };
}