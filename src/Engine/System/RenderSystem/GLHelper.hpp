#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include <iostream>

namespace neo {

    namespace GLHelper {

        const char * errorString(GLenum err) {
            switch (err) {
            case GL_NO_ERROR:
                return "No error";
            case GL_INVALID_ENUM:
                return "Invalid enum";
            case GL_INVALID_VALUE:
                return "Invalid value";
            case GL_INVALID_OPERATION:
                return "Invalid operation";
            case GL_STACK_OVERFLOW:
                return "Stack overflow";
            case GL_STACK_UNDERFLOW:
                return "Stack underflow";
            case GL_OUT_OF_MEMORY:
                return "Out of memory";
            default:
                return "No error";
            }
        }

        void printOpenGLErrors(char const * const Function, char const * const File, int const Line) {
            GLenum Error = glGetError();
            if (Error != GL_NO_ERROR)
            {
                const char *const ErrorString = errorString(Error);
                printf("OpenGL error in file '%s' at line %d calling function '%s': '%s' '%d 0x%X'\n", File, Line, Function, ErrorString, Error, Error);
            }
        }
    }

    #ifdef DEBUG_MODE
    #define CHECK_GL(x) do { GLHelper::printOpenGLErrors("{{BEFORE}} "#x, __FILE__, __LINE__); (x); GLHelper::printOpenGLErrors(#x, __FILE__, __LINE__); } while (0)
    #else
    #define CHECK_GL(x) (x)
    #endif
}