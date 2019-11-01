#include "GLHelper.hpp"

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
                if (str) {
                    printf("%s: ", str);
                }
                printf("GL_ERROR = %s.\n", errorString(glErr));
                assert(false);
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
                if (infoLog == NULL) {
                    puts("ERROR: Could not allocate InfoLog buffer");
                    std::cin.get();
                    assert(false);
                }
                glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
                printf("Shader InfoLog:\n%s\n\n", infoLog);
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
                if (infoLog == NULL) {
                    puts("ERROR: Could not allocate InfoLog buffer");
                    std::cin.get();
                    assert(false);
                }
                glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
                printf("Program InfoLog:\n%s\n\n", infoLog);
                free(infoLog);
            }
            checkError(GET_FILE_LINE);
        }

        void checkVersion() {
            int major = 0;
            int minor = 0;
            const char *verstr = (const char *)glGetString(GL_VERSION);

            if ((verstr == NULL) || (sscanf_s(verstr, "%d.%d", &major, &minor) != 2)) {
                printf("Invalid GL_VERSION format %d.%d\n", major, minor);
            }
            if (major < 2) {
                printf("This shader example will not work due to the installed Opengl version, which is %d.%d.\n", major, minor);
                assert(false);
            }
        }

        void printOpenGLErrors(char const * const Function, char const * const File, int const Line) {
            GLenum Error = glGetError();
            if (Error != GL_NO_ERROR)
            {
                const char *const ErrorString = errorString(Error);
                printf("OpenGL error in file '%s' at line %d calling function '%s': '%s' '%d 0x%X'\n", File, Line, Function, ErrorString, Error, Error);
                assert(false);
            }
        }

        void checkFrameBuffer() {
            GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (err != GL_FRAMEBUFFER_COMPLETE) {
                const char *const errString = errorString(err);
                printf("OpenGL error '%s' '%d 0x%X'\n", errString, err, err);
                assert(false);
            }
        }
    }
}