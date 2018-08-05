//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.
//    Modified by sueda 10/15/15.
//

#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include <sstream>
#include <iostream>
#include <cassert>
#include <stdio.h>

namespace neo {

        /* For printing out the current file and line number */
        template <typename T>
        std::string NumberToString(T x)
        {
        	std::ostringstream ss;
        	ss << x;
        	return ss.str();
        }
        #define GET_FILE_LINE (std::string(__FILE__) + ":" + NumberToString(__LINE__)).c_str()

        namespace GLHelper {

            void printOpenGLErrors(char const * const Function, char const * const File, int const Line);
            void printProgramInfoLog(GLuint program);
            void printShaderInfoLog(GLuint shader);
            void checkVersion();
        }

    #ifdef DEBUG_MODE
    #define CHECK_GL(x) do { GLHelper::printOpenGLErrors("{{BEFORE}} "#x, __FILE__, __LINE__); (x); GLHelper::printOpenGLErrors(#x, __FILE__, __LINE__); } while (0)
    #else
    #define CHECK_GL(x) (x)
    #endif
}
