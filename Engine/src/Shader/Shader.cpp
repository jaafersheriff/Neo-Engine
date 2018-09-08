#include "Shader.hpp"
#include "Util/Util.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include <fstream>
#include <vector>

namespace neo {
    Shader::Shader(const std::string &name) :
        name(name)
    {}

    Shader::Shader(const std::string &name, const std::string &v, const std::string &f) :
        Shader(name, v, f, "")
    {}

    Shader::Shader(const std::string &name, const std::string &v, const std::string &f, const std::string &g) :
        Shader(
            name,
            (v.size() ? Util::textFileRead((MasterRenderer::APP_SHADER_DIR + v).c_str()) : NULL), // memory leak
            (f.size() ? Util::textFileRead((MasterRenderer::APP_SHADER_DIR + f).c_str()) : NULL),
            (g.size() ? Util::textFileRead((MasterRenderer::APP_SHADER_DIR + g).c_str()) : NULL))
    {}

    Shader::Shader(const std::string &name, const char *vTex, const char *fTex) :
        Shader(name, vTex, fTex, NULL)
    {}

    Shader::Shader(const std::string &name, const char *vTex, const std::string &f) :
        Shader(name, vTex, (f.size() ? Util::textFileRead((MasterRenderer::APP_SHADER_DIR + f).c_str()) : NULL), NULL)
    {}

    Shader::Shader(const std::string &name, const char *vTex, const char *fTex, const char *gTex) :
        name(name) {
        pid = glCreateProgram();
        if (vTex && (vShaderId = compileShader(GL_VERTEX_SHADER, vTex))) {
            CHECK_GL(glAttachShader(pid, vShaderId));
        }
        if (fTex && (fShaderId = compileShader(GL_FRAGMENT_SHADER, fTex))) {
            CHECK_GL(glAttachShader(pid, fShaderId));
        }
        if (gTex && (gShaderId = compileShader(GL_GEOMETRY_SHADER, gTex))) {
            CHECK_GL(glAttachShader(pid, gShaderId));
        }
        CHECK_GL(glLinkProgram(pid));

        // See whether link was successful
        GLint linkSuccess;
        CHECK_GL(glGetProgramiv(pid, GL_LINK_STATUS, &linkSuccess));
        if (!linkSuccess) {
            GLHelper::printProgramInfoLog(pid);
            std::cout << "Error linking shader " << name << std::endl;
            std::cin.get();
            exit(EXIT_FAILURE);
        }

        if (vShaderId) {
            findAttributesAndUniforms(vTex);
        }
        if (fShaderId) {
            findAttributesAndUniforms(fTex);
        }
        if (gShaderId) {
            findAttributesAndUniforms(gTex);
        }

        std::cout << "Successfully compiled and linked " << name << std::endl;
    }

    GLuint Shader::compileShader(GLenum shaderType, const char *shaderString) {
        // Create the shader, assign source code, and compile it
        GLuint shader = glCreateShader(shaderType);
        CHECK_GL(glShaderSource(shader, 1, &shaderString, NULL));
        CHECK_GL(glCompileShader(shader));

        // See whether compile was successful
        GLint compileSuccess;
        CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess));
        if (!compileSuccess) {
            GLHelper::printShaderInfoLog(shader);
            std::cout << "Error compiling " << name;
            switch (shaderType) {
                case GL_VERTEX_SHADER:
                    std::cout << " vertex shader";
                    break;
                case GL_FRAGMENT_SHADER:
                    std::cout << " fragment shader";
                    break;
                case GL_GEOMETRY_SHADER:
                    std::cout << " geometry shader";
                    break;
                default:
                    break;
            }
            std::cout << std::endl;
            std::cin.get();
            exit(EXIT_FAILURE);
        }

        return shader;
    }

    void Shader::findAttributesAndUniforms(const char *shaderString) {
        char fileText[32768];
        strcpy(fileText, shaderString);
        std::vector<char *> lines;

        // Read the first line
        char * token = strtok(fileText, ";\n");
        lines.push_back(token);
        // Read all subsequent lines
        while ((token = strtok(NULL, ";\n")) != NULL) {
            lines.push_back(token);
        }

        // Look for keywords per line
        for (char *line : lines) {
            token = strtok(line, " (\n");
            if (token == NULL) {
                continue;
            }
            if (!strcmp(token, "uniform")) {
                // Handle lines with multiple variables separated by commas
                char *lineEnding = line + strlen(line) + 1;
                int lastDelimiter = -1;
                int lineEndingLength = strlen(lineEnding);
                for (int i = 0; i < lineEndingLength; i++) {
                    if (lineEnding[i] == ',') {
                        lineEnding[i] = '\0';
                        addUniform(lineEnding + (lastDelimiter + 1));
                        lastDelimiter = i;
                    }
                    else if (lineEnding[i] == ' ' || lineEnding[i] == '\t') {
                        lastDelimiter = i;
                    }
                }
                addUniform(lineEnding + (lastDelimiter + 1));
            }
            else if (!strcmp(token, "layout")) {
                char *lastToken = nullptr;
                while ((token = strtok(NULL, " ")) != NULL) {
                    lastToken = token;
                }
                if (lastToken) {
                    addAttribute(lastToken);
                }
            }
            else {
                continue;
            }
        }
    }

    void Shader::bind() {
        CHECK_GL(glUseProgram(pid));
    }

    void Shader::unbind() {
        CHECK_GL(glUseProgram(0));
    }

    void Shader::addAttribute(const std::string &name) {
        GLint r = glGetAttribLocation(pid, name.c_str());
        if (r < 0) {
            std::cerr << this->name << " WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        attributes[name] = r;
    }

    void Shader::addUniform(const std::string &name) {
        GLint r = glGetUniformLocation(pid, name.c_str());
        if (r < 0) {
            std::cerr << this->name << " WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        uniforms[name] = r;
    }

    GLint Shader::getAttribute(const std::string &name) const {
        std::map<std::string, GLint>::const_iterator attribute = attributes.find(name.c_str());
        if (attribute == attributes.end()) {
            std::cerr << name << " is not an attribute variable" << std::endl;
            return -1;
        }
        return attribute->second;
    }

    GLint Shader::getUniform(const std::string &name) const {
        std::map<std::string, GLint>::const_iterator uniform = uniforms.find(name.c_str());
        if (uniform == uniforms.end()) {
            std::cerr << name << " is not an uniform variable" << std::endl;
            return -1;
        }
        return uniform->second;
    }

    void Shader::cleanUp() {
        unbind();
        CHECK_GL(glDetachShader(pid, vShaderId));
        CHECK_GL(glDetachShader(pid, fShaderId));
        CHECK_GL(glDetachShader(pid, gShaderId));
        CHECK_GL(glDeleteShader(vShaderId));
        CHECK_GL(glDeleteShader(fShaderId));
        CHECK_GL(glDeleteShader(gShaderId));
        CHECK_GL(glDeleteProgram(pid));
    }

    void Shader::loadUniform(const std::string &loc, const bool b) const {
        CHECK_GL(glUniform1i(getUniform(loc), b));
    }

    void Shader::loadUniform(const std::string &loc, const int i) const {
        CHECK_GL(glUniform1i(getUniform(loc), i));
    }

    void Shader::loadUniform(const std::string &loc, const GLuint i) const {
        CHECK_GL(glUniform1i(getUniform(loc), i));
    }

    void Shader::loadUniform(const std::string &loc, const float f) const {
        CHECK_GL(glUniform1f(getUniform(loc), f));
    }

    void Shader::loadUniform(const std::string &loc, const glm::vec2 & v) const {
        CHECK_GL(glUniform2f(getUniform(loc), v.x, v.y));
    }

    void Shader::loadUniform(const std::string &loc, const glm::vec3 & v) const {
        CHECK_GL(glUniform3f(getUniform(loc), v.x, v.y, v.z));
    }

    void Shader::loadUniform(const std::string &loc, const glm::vec4 & v) const {
        CHECK_GL(glUniform4f(getUniform(loc), v.r, v.g, v.b, v.a));
    }

    void Shader::loadUniform(const std::string &loc, const glm::mat3 & m) const {
        CHECK_GL(glUniformMatrix3fv(getUniform(loc), 1, GL_FALSE, glm::value_ptr(m)));
    }

    void Shader::loadUniform(const std::string &loc, const glm::mat4 & m) const {
        CHECK_GL(glUniformMatrix4fv(getUniform(loc), 1, GL_FALSE, glm::value_ptr(m)));
    }
}