#include "Renderer/pch.hpp"

#include "ResolvedShaderInstance.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/NewShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Loader/Library.hpp"

namespace neo {
    namespace {

        static std::string _processShader(const char* shaderString, const NewShader::ShaderDefines& defines) {
            if (!shaderString) {
                return "";
            }
            std::string sourceString(shaderString);

            // Prepend #version
            sourceString.insert(0, (ServiceLocator<Renderer>::ref().mDetails.mGLSLVersion + "\n").c_str());

            // Handle #includes 
            {
                std::string::size_type start = 0;
                std::string::size_type end = 0;
                while ((end = sourceString.find("\n", start)) != std::string::npos) {
                    std::string line = sourceString.substr(start, end - start);

                    // Find name
                    std::string::size_type nameStart = line.find("#include \"");
                    std::string::size_type nameEnd = line.find("\"", nameStart + 10);
                    if (nameStart != std::string::npos && nameEnd != std::string::npos && nameStart != nameEnd) {
                        const char* sourceInclude = Loader::loadFileString(line.substr(nameStart + 10, nameEnd - nameStart - 10).c_str());

                        // Replace include with source
                        sourceString.erase(start, end - start);
                        sourceString.insert(start, sourceInclude);
                        delete sourceInclude;

                        // Reset processing incase there are internal #includes
                        start = end = 0;

                    }
                    start = end + 1;
                }
            }

            // Handle #defines
            {
                // TODO
                NEO_UNUSED(defines);
            }

            return sourceString;
        }

        static void _findUniforms(const char *shaderString, std::vector<std::string>& uniforms) {
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
                    size_t lineEndingLength = strlen(lineEnding);
                    for (int i = 0; i < lineEndingLength; i++) {
                        if (lineEnding[i] == ',') {
                            lineEnding[i] = '\0';
                            uniforms.push_back(lineEnding + (lastDelimiter + 1));
                            lastDelimiter = i;
                        }
                        else if (lineEnding[i] == ' ' || lineEnding[i] == '\t') {
                            lastDelimiter = i;
                        }
                    }
                    uniforms.push_back(lineEnding + (lastDelimiter + 1));
                }
                // Attributes, if you want
                // else if (!strcmp(token, "layout")) {
                //     char *lastToken = nullptr;
                //     while ((token = strtok(NULL, " ")) != NULL) {
                //         lastToken = token;
                //     }
                //     if (lastToken) {
                //         _addAttribute(HashedString(lastToken));
                //     }
                // }
                else {
                    continue;
                }
            }
        }
    }

    bool ResolvedShaderInstance::init(const NewShader::ConstructionArgs& args, const NewShader::ShaderDefines& defines) {
        NEO_ASSERT(!mValid && mPid == 0, "TODO");
        mValid = false;
        mPid = glCreateProgram();

        for (auto&& [stage, source] : args) {
            if (!source) {
                return mValid;
            }
            std::string processedSource = _processShader(source, defines);
            if (processedSource.size()) {
                mShaderIDs[stage] = _compileShader(GLHelper::getGLShaderStage(stage), processedSource.c_str());
                if (mShaderIDs[stage]) {
                    glAttachShader(mPid, mShaderIDs[stage]);

                    std::vector<std::string> uniforms;
                    _findUniforms(processedSource.c_str(), uniforms);
                    for (auto& uniform : uniforms) {
                        GLint r = glGetUniformLocation(mPid, uniform.c_str());
                        if (r < 0) {
                            NEO_LOG_S(util::LogSeverity::Warning, "WARN: %s uniform cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it", uniform.c_str());
                        }
                        mUniforms[HashedString(uniform.c_str())] = r;
                    }
                }
            }
        }
        glLinkProgram(mPid);

        // See whether link was successful
        GLint linkSuccess;
        glGetProgramiv(mPid, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess) {
            GLHelper::printProgramInfoLog(mPid);
        }

        mValid = true;
        return mValid;
    }

    GLuint ResolvedShaderInstance::_compileShader(GLenum shaderType, const char *shaderString) {
        // Create the shader, assign source code, and compile it
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderString, NULL);
        glCompileShader(shader);

        // See whether compile was successful
        GLint compileSuccess;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
        if (!compileSuccess) {
            GLHelper::printShaderInfoLog(shader);
        }

        return shader;
    }

    void ResolvedShaderInstance::destroy() {
        if (mPid) {
            unbind();
            for (auto&& [stage, id] : mShaderIDs) {
                glDetachShader(mPid, id);
                glDeleteShader(id);
            }
            glDeleteProgram(mPid);
        }
        mPid = 0;
    }

    void ResolvedShaderInstance::bind() const {
        glUseProgram(mPid);
    }

    void ResolvedShaderInstance::unbind() const {
        glUseProgram(0);
    }

    GLint ResolvedShaderInstance::_getUniform(const char* name) const {
        ServiceLocator<Renderer>::ref().mStats.mNumUniforms++;
        const auto uniform = mUniforms.find(HashedString(name));
        if (uniform == mUniforms.end()) {
            // NEO_LOG_S(util::LogSeverity::Warning, "%s is not an uniform variable", name);
            return -1;
        }
        return uniform->second;
    }

    void ResolvedShaderInstance::bindUniform(const char* name, const UniformVariant& uniform) const {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, bool>)
                glUniform1i(_getUniform(name), arg);
            else if constexpr (std::is_same_v<T, int>)
                glUniform1i(_getUniform(name), arg);
            else if constexpr (std::is_same_v<T, uint32_t>)
                glUniform1ui(_getUniform(name), arg);
            else if constexpr (std::is_same_v<T, double>)
                glUniform1f(_getUniform(name), static_cast<float>(arg));
            else if constexpr (std::is_same_v<T, float>)
                glUniform1f(_getUniform(name), arg);
            else if constexpr (std::is_same_v<T, glm::vec2>)
                glUniform2f(_getUniform(name), arg.x, arg.y);
            else if constexpr (std::is_same_v<T, glm::ivec2>)
                glUniform2i(_getUniform(name), arg.x, arg.y);
            else if constexpr (std::is_same_v<T, glm::vec3>)
				glUniform3f(_getUniform(name), arg.x, arg.y, arg.z);
            else if constexpr (std::is_same_v<T, glm::vec4>)
                glUniform4f(_getUniform(name), arg.x, arg.y, arg.z, arg.w);
            else if constexpr (std::is_same_v<T, glm::mat3>)
                glUniformMatrix3fv(_getUniform(name), 1, GL_FALSE, &arg[0][0]);
            else if constexpr (std::is_same_v<T, glm::mat4>)
                glUniformMatrix4fv(_getUniform(name), 1, GL_FALSE, &arg[0][0]);
            else
                static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }, uniform);
    }

    void ResolvedShaderInstance::bindTexture(const char* name, const Texture& texture) const {
        ServiceLocator<Renderer>::ref().mStats.mNumSamplers++;
        texture.bind();
        glUniform1i(_getUniform(name), texture.mTextureID);

    }
}