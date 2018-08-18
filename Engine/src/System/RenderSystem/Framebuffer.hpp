#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "Util/GLHelper.hpp"

#include "Model/Texture.hpp"

namespace neo {

    class Framebuffer {
    
        public: 
            GLuint fboId;

            virtual ~Framebuffer() {
                CHECK_GL(glDeleteFramebuffers(1, &fboId));
            }

            void generate() {
                CHECK_GL(glGenFramebuffers(1, &fboId));
            }

            void bind() {
                CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fboId));
            }

            // TODO - ability to add multiple color attachments 
            // TODO - ability to add color render buffers
            void attachColorTexture(const Texture &texture) {
                attachTexture(GL_COLOR_ATTACHMENT0, texture.textureId);
            }

            void attachDepthTexture(const Texture &texture) {
                attachTexture(GL_DEPTH_ATTACHMENT, texture.textureId);
            }

        private:
            void attachTexture(GLuint component, GLuint id) {
                bind();
                CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, component, GL_TEXTURE_2D, id, 0));
                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    CHECK_GL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
                }
            }
    };
}