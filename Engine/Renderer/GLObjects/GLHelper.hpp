//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.
//    Modified by sueda 10/15/15.
//

#pragma once

#include <GL/glew.h>
#include <string>

namespace neo {

    enum class ShaderStage {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        TESSELLATION_CONTROL,
        TESSELLATION_EVAL,
        COMPUTE
    };

    namespace GLHelper {

        void OpenGLMessageCallback(
            unsigned source,
            unsigned type,
            unsigned id,
            unsigned severity,
            int length,
            const char* message,
            const void* userParam
        );
        GLint getGLShaderStage(ShaderStage type);
        void checkFrameBuffer();
        void printProgramInfoLog(GLuint program);
        void printShaderInfoLog(GLuint shader);
    }

#ifdef DEBUG_MODE
#define CHECK_GL_FRAMEBUFFER() do {GLHelper::checkFrameBuffer(); } while(0)
#else
#define CHECK_GL_FRAMEBUFFER()
#endif
}
