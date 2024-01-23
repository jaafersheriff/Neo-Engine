#include "Renderer/pch.hpp"
#include "GLHelper.hpp"

namespace neo {

    namespace GLHelper {
        namespace {

            /* For printing out the current file and line number */
            template <typename T>
            std::string NumberToString(T x)
            {
                std::ostringstream ss;
                ss << x;
                return ss.str();
            }

        }

        void OpenGLMessageCallback(
            unsigned source,
            unsigned type,
            unsigned id,
            unsigned severity,
            int length,
            const char* message,
            const void* userParam) {
            NEO_UNUSED(id, length, userParam);

            static std::unordered_map<GLenum, const char*> sSourceString = {
                {GL_DEBUG_SOURCE_API, "API"},
                {GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Window"},
                {GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader Compiler"},
                {GL_DEBUG_SOURCE_THIRD_PARTY, "3rd Party"},
                {GL_DEBUG_SOURCE_APPLICATION, "Application"},
                {GL_DEBUG_SOURCE_OTHER, "Other"},
            };

            static std::unordered_map<GLenum, const char*> sTypeString{
                {GL_DEBUG_TYPE_ERROR, "Error"},
                {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated Behavior"},
                {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "Undefined Behavior"},
                {GL_DEBUG_TYPE_PORTABILITY, "Portability"},
                {GL_DEBUG_TYPE_PERFORMANCE, "Performance"},
                {GL_DEBUG_TYPE_MARKER, "Marker"},
                {GL_DEBUG_TYPE_PUSH_GROUP, "Push Group"},
                {GL_DEBUG_TYPE_POP_GROUP, "Pop Group"},
                {GL_DEBUG_TYPE_OTHER, "Other"},
            };

            char glBuf[512];
            sprintf(glBuf, "[GL %s] [%s]: %s", sSourceString.at(source), sTypeString.at(type), message);

            switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH:         NEO_LOG_E(glBuf); return;
                case GL_DEBUG_SEVERITY_MEDIUM:       NEO_LOG_W(glBuf); return;
                case GL_DEBUG_SEVERITY_LOW:          NEO_LOG_W(glBuf); return;
                case GL_DEBUG_SEVERITY_NOTIFICATION: NEO_LOG_I(glBuf); return;
            }
            NEO_LOG_E(glBuf);
            NEO_FAIL("Unknown severity level!");
        }

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

        GLint getGLShaderStage(ShaderStage type) {
            switch(type) {
            case(ShaderStage::VERTEX):
                return GL_VERTEX_SHADER;
            case(ShaderStage::FRAGMENT):
                return GL_FRAGMENT_SHADER;
            case(ShaderStage::GEOMETRY):
                return GL_GEOMETRY_SHADER;
            case(ShaderStage::COMPUTE):
                return GL_COMPUTE_SHADER;
            case(ShaderStage::TESSELLATION_CONTROL):
                return GL_TESS_CONTROL_SHADER;
            case(ShaderStage::TESSELLATION_EVAL):
                return GL_TESS_EVALUATION_SHADER;
            }
            NEO_FAIL("Invalid ShaderStage: %d", type);
            return 0;
        }

        void checkError(const char *str) {
            GLenum glErr = glGetError();
            if (glErr != GL_NO_ERROR) {
                NEO_LOG_E("GL_ERROR at %s : %s.\n", str, errorString(glErr));
            }
        }

        void printShaderInfoLog(GLuint shader) {
            GLint infologLength = 0;
            GLint charsWritten = 0;
            GLchar *infoLog;

            #define GET_FILE_LINE (std::string(__FILE__) + ":" + NumberToString(__LINE__)).c_str()
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