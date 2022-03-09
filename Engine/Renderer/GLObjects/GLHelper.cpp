#include "GLHelper.hpp"

#include "Util/Util.hpp"
#include "Util/Log/Log.hpp"

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
            case GL_FRAMEBUFFER_UNDEFINED:
                return "Framebuffer undefined";
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                return "Incomplete attachment";
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                return "Incomplete or missing attachment";
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                return "Incomplete draw buffer";
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                return "Incomplete read buffer";
            case GL_FRAMEBUFFER_UNSUPPORTED:
                return "Framebuffer unsupported";
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                return "Incomplete multisample";
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                return "Incomplete layer targets";
            default:
                return "No error";
            }
        }

        void checkError(const char *str) {
            GLenum glErr = glGetError();
            if (glErr != GL_NO_ERROR) {
                NEO_FAIL("GL_ERROR at %s : %s.\n", str, errorString(glErr));
            }
        }

        void printShaderInfoLog(GLuint shader) {
            GLint infologLength = 0;
            GLint charsWritten = 0;
            GLchar *infoLog;

            checkError(GET_FILE_LINE);
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
            checkError(GET_FILE_LINE);

            if (infologLength > 0) {
                infoLog = (GLchar *)malloc(infologLength);
                NEO_ASSERT(infoLog != NULL, "Could not allocate InfoLog buffer");
                glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
                NEO_LOG_E("Shader InfoLog:\n%s\n\n", infoLog);
                free(infoLog);
            }
            checkError(GET_FILE_LINE);
        }

        void printProgramInfoLog(GLuint program) {
            GLint infologLength = 0;
            GLint charsWritten = 0;
            GLchar *infoLog;

            checkError(GET_FILE_LINE);
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
            checkError(GET_FILE_LINE);

            if (infologLength > 0) {
                infoLog = (GLchar *)malloc(infologLength);
                NEO_ASSERT(infoLog != NULL, "Could not allocate InfoLog buffer");
                glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
                NEO_LOG_E("Program InfoLog:\n%s\n\n", infoLog);
                free(infoLog);
            }
            checkError(GET_FILE_LINE);
        }

        void checkFrameBuffer() {
            GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (err != GL_FRAMEBUFFER_COMPLETE) {
                const char *const errString = errorString(err);
                NEO_FAIL("OpenGL error '%s' '%d 0x%X'\n", errString, err, err);
            }
        }
    }
}