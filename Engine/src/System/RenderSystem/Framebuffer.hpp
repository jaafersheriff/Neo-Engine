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
                GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                if (err != GL_FRAMEBUFFER_COMPLETE) {
                    std::string errString;
                    switch (err) {
                    case GL_FRAMEBUFFER_UNDEFINED:
                        errString = "Framebuffer undefined";
                    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                        errString = "Incomplete attachment";
                    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                        errString = "Incomplete or missing attachment";
                    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                        errString = "Incomplete draw buffer";
                    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                        errString = "Incomplete read buffer";
                    case GL_FRAMEBUFFER_UNSUPPORTED:
                        errString = "Framebuffer unsupported";
                    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                        errString = "Incomplete multisample";
                    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                        errString = "Incomplete layer targets";
                    default:
                        errString = "Framebuffer undefined";
                    }
                    printf("Framebuffer error: '%s'\n", errString.c_str());
                }
            }
    };
}