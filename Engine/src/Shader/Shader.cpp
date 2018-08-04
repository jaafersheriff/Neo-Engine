#include "Shader.hpp"
#include "GLHelper.hpp"

#include <fstream>
#include <vector>

namespace neo {

    Shader::Shader(const std::string &name, const std::string &dir, const std::string &v, const std::string &f) :
        Shader(name, dir, v, f, "")
    { }

    Shader::Shader(const std::string &name, const std::string &dir, const std::string &vName, const std::string &fName, const std::string &gName) :
        name(name),
        vShaderName(vName),
        fShaderName(fName),
        gShaderName(gName) {
        pid = glCreateProgram();
        if (vShaderName.size() && (vShaderId = compileShader(GL_VERTEX_SHADER, dir, vShaderName))) {
            CHECK_GL(glAttachShader(pid, vShaderId));
        }
        if (fShaderName.size() && (fShaderId = compileShader(GL_FRAGMENT_SHADER, dir, fShaderName))) {
            CHECK_GL(glAttachShader(pid, fShaderId));
        }
        if (gShaderName.size() && (gShaderId = compileShader(GL_GEOMETRY_SHADER, dir, gShaderName))) {
            CHECK_GL(glAttachShader(pid, gShaderId));
        }
        CHECK_GL(glLinkProgram(pid));

        // See whether link was successful
        GLint linkSuccess;
        CHECK_GL(glGetProgramiv(pid, GL_LINK_STATUS, &linkSuccess));
        if (!linkSuccess) {
            GLHelper::printProgramInfoLog(pid);
            std::cout << "Error linking shaders " << vShaderName << " and " << fShaderName;
            if (gShaderId) {
                std::cout << " and " << gShaderName << std::endl;
            }
            else {
                std::cout << std::endl;
            }
            std::cin.get();
            exit(EXIT_FAILURE);
        }

        if (vShaderId) {
            findAttributesAndUniforms(dir, vShaderName);
        }
        if (fShaderId) {
            findAttributesAndUniforms(dir, fShaderName);
        }
        if (gShaderId) {
            findAttributesAndUniforms(dir, gShaderName);
        }
    }

    GLuint Shader::compileShader(GLenum shaderType, const std::string &dir, const std::string &shaderName) {
        // Read the shader source file into a string
        char *shaderString = GLHelper::textFileRead((dir + shaderName).c_str());
        // Stop if there was an error reading the shader source file
        if (shaderString == NULL) {
            std::cout << "Could not read shader: " << dir << shaderName << std::endl;
            std::cin.get();
            exit(EXIT_FAILURE);
        }

        // Create the shader, assign source code, and compile it
        GLuint shader = glCreateShader(shaderType);
        CHECK_GL(glShaderSource(shader, 1, &shaderString, NULL));
        CHECK_GL(glCompileShader(shader));

        // See whether compile was successful
        GLint compileSuccess;
        CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess));
        if (!compileSuccess) {
            GLHelper::printShaderInfoLog(shader);
            std::cout << "Error compiling shader: " << dir << shaderName << std::endl;
            std::cin.get();
            exit(EXIT_FAILURE);
        }

        // Free the memory
        free(shaderString);

        return shader;
    }

    void Shader::findAttributesAndUniforms(const std::string &res, const std::string &shaderName) {
        char *fileText = GLHelper::textFileRead((res + shaderName).c_str());
        char *token;
        char *lastToken = nullptr;

        std::vector<char *> lines;

        // Read the first line
        token = strtok(fileText, ";\n");
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

        // Free the memory
        free(fileText);
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
            std::cerr << "WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        attributes[name] = r;
    }

    void Shader::addUniform(const std::string &name) {
        GLint r = glGetUniformLocation(pid, name.c_str());
        if (r < 0) {
            std::cerr << "WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        uniforms[name] = r;
    }

    GLint Shader::getAttribute(const std::string &name) {
        std::map<std::string, GLint>::const_iterator attribute = attributes.find(name.c_str());
        if (attribute == attributes.end()) {
            std::cerr << name << " is not an attribute variable" << std::endl;
            return -1;
        }
        return attribute->second;
    }

    GLint Shader::getUniform(const std::string &name) {
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

    void Shader::loadBool(const int location, const bool b) const {
        CHECK_GL(glUniform1i(location, b));
    }

    void Shader::loadInt(const int location, const int i) const {
        CHECK_GL(glUniform1i(location, i));
    }

    void Shader::loadFloat(const int location, const float f) const {
        CHECK_GL(glUniform1f(location, f));
    }

    void Shader::loadVector(const int location, const glm::vec2 & v) const {
        CHECK_GL(glUniform2f(location, v.x, v.y));
    }

    void Shader::loadVector(const int location, const glm::vec3 & v) const {
        CHECK_GL(glUniform3f(location, v.x, v.y, v.z));
    }

    void Shader::loadVector(const int location, const glm::vec4 & v) const {
        CHECK_GL(glUniform4f(location, v.r, v.g, v.b, v.a));
    }

    void Shader::loadMatrix(const int location, const glm::mat3 & m) const {
        CHECK_GL(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(m)));
    }

    void Shader::loadMatrix(const int location, const glm::mat4 & m) const {
        CHECK_GL(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m)));
    }
}