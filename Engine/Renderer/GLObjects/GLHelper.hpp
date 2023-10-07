//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.
//    Modified by sueda 10/15/15.
//

#pragma once

#include "GL/glew.h"

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
