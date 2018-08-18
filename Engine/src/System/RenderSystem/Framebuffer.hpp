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
            // TODO - if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            void attachColorTexture(const Texture &texture) {
                bind();
                CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.textureId, 0));
            }
    };
}