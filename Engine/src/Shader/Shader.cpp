#include "Shader.hpp"
#include "Util/Util.hpp"
#include "GLObjects/Texture.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Renderer/Renderer.hpp"

#include <fstream>
#include <vector>

namespace neo {
    Shader::Shader(const std::string &name) :
        mName(name)
    {}

    Shader::Shader(const std::string &name, const std::string& vertexFile, const std::string& fragmentFile) :
        Shader(name) {
        _attachStage(ShaderStage::VERTEX, vertexFile);
        _attachStage(ShaderStage::FRAGMENT, fragmentFile);
        init();
    }

    Shader::Shader(const std::string &name, const char* vertexSource, const char* fragmentSource) :
        Shader(name) {
        _attachStage(ShaderStage::VERTEX, vertexSource);
        _attachStage(ShaderStage::FRAGMENT, fragmentSource);
        init();
    }

    void Shader::reload() {
        cleanUp();
        for (auto &&[stage, source] : mStages) {
            if (source.file.size()) {
                delete source.source;
                source.source = Util::textFileRead(_getFullPath(source.file).c_str());
            }
        }
        init();
    }

    void Shader::_attachStage(ShaderStage stage, const std::string& file) {
        mStages.emplace(stage, ShaderSource{ 0, file, Util::textFileRead(_getFullPath(file).c_str()) });
    }

    void Shader::_attachStage(ShaderStage stage, const char* source) {
        mStages.emplace(stage, ShaderSource{ 0, "", source });
    }

    void Shader::init() {
        mPID = glCreateProgram();
        
        for (auto &&[stage, source] : mStages) {
            source.processedSource = _processShader(source.source);
            if (source.processedSource.size() && (source.id = _compileShader(_getGLShaderStage(stage), source.processedSource.c_str()))) {
                CHECK_GL(glAttachShader(mPID, source.id));
            }
        }

        CHECK_GL(glLinkProgram(mPID));

        // See whether link was successful
        GLint linkSuccess;
        CHECK_GL(glGetProgramiv(mPID, GL_LINK_STATUS, &linkSuccess));
        if (!linkSuccess) {
            GLHelper::printProgramInfoLog(mPID);
            printf("Error linking shader %s\n", mName.c_str());
            NEO_ASSERT(false, "");
        }

        for (auto &&[type, source] : mStages) {
            if (source.id) {
                _findAttributesAndUniforms(source.processedSource.c_str());
            }
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
        sourceString.insert(0, (Renderer::NEO_GLSL_VERSION + "\n").c_str());

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
            printf("%s shader file doesn't exist %s\n", fileName.c_str());
            NEO_ASSERT(false, "");
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
                case GL_COMPUTE_SHADER:
                    std::cout << " compute shader";
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

            NEO_ASSERT(false, "");
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
                        _addUniform(lineEnding + (lastDelimiter + 1));
                        lastDelimiter = i;
                    }
                    else if (lineEnding[i] == ' ' || lineEnding[i] == '\t') {
                        lastDelimiter = i;
                    }
                }
                _addUniform(lineEnding + (lastDelimiter + 1));
            }
            else if (!strcmp(token, "layout")) {
                char *lastToken = nullptr;
                while ((token = strtok(NULL, " ")) != NULL) {
                    lastToken = token;
                }
                if (lastToken) {
                    _addAttribute(lastToken);
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

    void Shader::_addAttribute(const std::string &name) {
        GLint r = glGetAttribLocation(mPID, name.c_str());
        if (r < 0) {
            std::cerr << this->mName << " WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        mAttributes[name] = r;
    }

    void Shader::_addUniform(const std::string &name) {
        GLint r = glGetUniformLocation(mPID, name.c_str());
        if (r < 0) {
            std::cerr << this->mName << " WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
        }
        mUniforms[name] = r;
    }

    GLint Shader::getAttribute(const std::string &name) const {
        std::map<std::string, GLint>::const_iterator attribute = mAttributes.find(name.c_str());
        if (attribute == mAttributes.end()) {
            std::cerr << name << " is not an attribute variable - did you remember to call Shader::init()" << std::endl;
            return -1;
        }
        return attribute->second;
    }

    GLint Shader::getUniform(const std::string &name) const {
        std::map<std::string, GLint>::const_iterator uniform = mUniforms.find(name.c_str());
        if (uniform == mUniforms.end()) {
            std::cerr << name << " is not an uniform variable - did you remember to call Shader::init()" << std::endl;
            return -1;
        }
        return uniform->second;
    }

    void Shader::cleanUp() {
        if (mPID) {
            unbind();
            for (auto &&[type, source] : mStages) {
                CHECK_GL(glDetachShader(mPID, source.id));
                CHECK_GL(glDeleteShader(source.id));
                source.id = 0;
            }
        }
        CHECK_GL(glDeleteProgram(mPID));
        mPID = 0;
    }

    GLint Shader::_getGLShaderStage(ShaderStage type) {
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
        NEO_ASSERT(false, "Invalid ShaderStage");
    }

    void Shader::loadUniform(const std::string &loc, const bool b) const {
        CHECK_GL(glUniform1i(getUniform(loc), b));
    }

    void Shader::loadUniform(const std::string &loc, const int i) const {
        CHECK_GL(glUniform1i(getUniform(loc), i));
    }

    void Shader::loadUniform(const std::string &loc, const double d) const {
        CHECK_GL(glUniform1f(getUniform(loc), static_cast<float>(d)));
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

    void Shader::loadTexture(const std::string &loc, const Texture & texture) const {
        texture.bind();
        CHECK_GL(glUniform1i(getUniform(loc), texture.mTextureID));
    }

}