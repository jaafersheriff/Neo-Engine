#include "Shader.hpp"
#include "Util/Util.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Renderer/Renderer.hpp"

#include <fstream>
#include <vector>

namespace neo {
    Shader::Shader(const std::string &name) :
        mName(name)
    {}

    Shader::Shader(const std::string &name, const std::string &vName, const std::string &fName) :
        Shader(name, vName, fName, "")
    {}

    Shader::Shader(const std::string &name, const std::string &vName, const std::string &fName, const std::string &gName) :
        Shader(
            name,
            (vName.size() ? Util::textFileRead(_getFullPath(vName).c_str()) : NULL),
            (fName.size() ? Util::textFileRead(_getFullPath(fName).c_str()) : NULL),
            (gName.size() ? Util::textFileRead(_getFullPath(gName).c_str()) : NULL))
    {}

    Shader::Shader(const std::string &name, const char *vTex, const char *fTex) :
        Shader(name, vTex, fTex, NULL)
    {}

    Shader::Shader(const std::string &name, const char *vTex, const std::string &fName) :
        Shader(name, vTex, (fName.size() ? Util::textFileRead(_getFullPath(fName).c_str()) : NULL), NULL)
    {}

    Shader::Shader(const std::string &name, const char *vTex, const char *fTex, const char *gTex) :
        mName(name),
        mVertexSource(vTex),
        mFragmentSource(fTex),
        mGeometrySource(gTex) {

        _init();
   }

    void Shader::reload() {
        cleanUp();
        _init();
    }

    void Shader::_init() {
        mPID = glCreateProgram();

        std::string processedVertex = _processShader(mVertexSource);
        std::string processedFragment = _processShader(mFragmentSource);
        std::string processedGeometry = _processShader(mGeometrySource);

        if (processedVertex.size() && (mVertexID = _compileShader(GL_VERTEX_SHADER, processedVertex.c_str()))) {
            CHECK_GL(glAttachShader(mPID, mVertexID));
        }
        if (processedFragment.size() && (mFragmentID = _compileShader(GL_FRAGMENT_SHADER, processedFragment.c_str()))) {
            CHECK_GL(glAttachShader(mPID, mFragmentID));
        }
        if (processedGeometry.size() && (mGeometryID = _compileShader(GL_GEOMETRY_SHADER, processedGeometry.c_str()))) {
            CHECK_GL(glAttachShader(mPID, mGeometryID));
        }
        CHECK_GL(glLinkProgram(mPID));

        // See whether link was successful
        GLint linkSuccess;
        CHECK_GL(glGetProgramiv(mPID, GL_LINK_STATUS, &linkSuccess));
        if (!linkSuccess) {
            GLHelper::printProgramInfoLog(mPID);
            std::cout << "Error linking shader " << mName << std::endl;
            std::cin.get();
            exit(EXIT_FAILURE);
        }

        if (mVertexID) {
            _findAttributesAndUniforms(processedVertex.c_str());
        }
        if (mFragmentID) {
            _findAttributesAndUniforms(processedFragment.c_str());
        }
        if (mGeometryID) {
            _findAttributesAndUniforms(processedGeometry.c_str());
        }

        std::cout << "Successfully compiled and linked " << mName << std::endl;
    }

    // Handle #includes
    std::string Shader::_processShader(const char *shaderString) {
        if (!shaderString) {
            return "";
        }

        std::string sourceString(shaderString);

        // Prepend #version
        sourceString.insert(0, "#version 330 core\n");

        // Break up source by line
        std::string::size_type start = 0;
        std::string::size_type end = 0;
        while ((end = sourceString.find("\n", start)) != std::string::npos) {
            std::string line = sourceString.substr(start, end - start);

            // Find name
            std::string::size_type nameStart = line.find("#include \"");
            std::string::size_type nameEnd = line.find("\"", nameStart + 10);
            if (nameStart != std::string::npos && nameEnd != std::string::npos && nameStart != nameEnd) {
                std::string fileName = _getFullPath(line.substr(nameStart + 10, nameEnd - nameStart - 10));

                // Load file
                char* sourceInclude = Util::textFileRead(fileName.c_str());

                // Replace include with source
                sourceString.erase(start, end - start);
                sourceString.insert(start, sourceInclude);
                delete sourceInclude;

                // Reset processing incase there are internal #includes
                start = end = 0;

            }
            start = end + 1;
        }

        return sourceString;
    }

    std::string Shader::_getFullPath(const std::string& fileName) {
        if (Util::fileExists((Renderer::APP_SHADER_DIR + fileName).c_str())) {
            return Renderer::APP_SHADER_DIR + fileName;
        }
        else if (Util::fileExists(("../Engine/shaders/" + fileName).c_str())) {
            return "../Engine/shaders/" + fileName;
        }
        else {
            assert(false, fileName + "doesn't exist");
        }
    }

    GLuint Shader::_compileShader(GLenum shaderType, const char *shaderString) {
        // Create the shader, assign source code, and compile it
        GLuint shader = glCreateShader(shaderType);
        CHECK_GL(glShaderSource(shader, 1, &shaderString, NULL));
        CHECK_GL(glCompileShader(shader));

        // See whether compile was successful
        GLint compileSuccess;
        CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess));
        if (!compileSuccess) {
            GLHelper::printShaderInfoLog(shader);
            std::cout << "Error compiling " << mName;
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

            std::stringstream ss(shaderString);
            std::string line;
            int lineNum = 1;
            while (shaderString && std::getline(ss, line, '\n')) {
                std::cout << lineNum++ << " " << line << std::endl;
            }

            std::cin.get();
            exit(EXIT_FAILURE);
        }

        return shader;
    }

    void Shader::_findAttributesAndUniforms(const char *shaderString) {
        char fileText[1<<16];
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
        CHECK_GL(glUseProgram(mPID));
    }

    void Shader::unbind() {
        CHECK_GL(glUseProgram(0));
    }

    void Shader::addAttribute(const std::string &name) {
        GLint r = glGetAttribLocation(mPID, name.c_str());
        if (r < 0) {
            std::cerr << this->mName << " WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        mAttributes[name] = r;
    }

    void Shader::addUniform(const std::string &name) {
        GLint r = glGetUniformLocation(mPID, name.c_str());
        if (r < 0) {
            std::cerr << this->mName << " WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        mUniforms[name] = r;
    }

    GLint Shader::getAttribute(const std::string &name) const {
        std::map<std::string, GLint>::const_iterator attribute = mAttributes.find(name.c_str());
        if (attribute == mAttributes.end()) {
            std::cerr << name << " is not an attribute variable" << std::endl;
            return -1;
        }
        return attribute->second;
    }

    GLint Shader::getUniform(const std::string &name) const {
        std::map<std::string, GLint>::const_iterator uniform = mUniforms.find(name.c_str());
        if (uniform == mUniforms.end()) {
            std::cerr << name << " is not an uniform variable" << std::endl;
            return -1;
        }
        return uniform->second;
    }

    void Shader::cleanUp() {
        if (mPID) {
            unbind();
            if (mVertexID) {
                CHECK_GL(glDetachShader(mPID, mVertexID));
                CHECK_GL(glDeleteShader(mVertexID));

            }
            if (mFragmentID) {
                CHECK_GL(glDetachShader(mPID, mFragmentID));
                CHECK_GL(glDeleteShader(mFragmentID));
            }
            if (mGeometryID) {
                CHECK_GL(glDeleteShader(mGeometryID));
                CHECK_GL(glDetachShader(mPID, mGeometryID));
            }
        }
        CHECK_GL(glDeleteProgram(mPID));
        mPID = mVertexID = mFragmentID = mGeometryID = 0;
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