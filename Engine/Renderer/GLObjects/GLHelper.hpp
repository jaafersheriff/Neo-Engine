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

#define GPU_MP_ENTERD(define, group, name) \
    if (glIsEnabled(GL_DEBUG_OUTPUT)) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(name.size()), name.c_str()); \
    MICROPROFILE_DEFINE(define, group, name.c_str(), MP_AUTO);\
    MICROPROFILE_ENTER(define);\
    MICROPROFILE_DEFINE_GPU(define, name.c_str(),  MP_AUTO);\
    MICROPROFILE_GPU_ENTER(define)

#define GPU_MP_ENTER(name) \
    if (glIsEnabled(GL_DEBUG_OUTPUT)) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name); \
    MICROPROFILE_ENTERI("Renderer", name, MP_AUTO);\
    MICROPROFILE_GPU_ENTERI("RendererGPU", name, MP_AUTO)

#define GPU_MP_LEAVE() \
    if (glIsEnabled(GL_DEBUG_OUTPUT)) glPopDebugGroup(); \
    MICROPROFILE_LEAVE();\
    MICROPROFILE_GPU_LEAVE()
}
