#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "Util/GLHelper.hpp"

namespace neo {

    class Framebuffer {
    
        public: 
            GLuint fboId;

            void bind() {
                CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fboId));
            }
    };
}