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
                std::string::size_type start = 0;
                std::string::size_type end = 0;
                while ((end = sourceString.find("\n", start)) != std::string::npos) {
                    std::string line = sourceString.substr(start, end - start);

                    std::string::size_type defineStart = line.find("#ifdef ");
                    if (defineStart != std::string::npos && end != std::string::npos && defineStart != end) {
                        std::string define = line.substr(defineStart + 7, end - start - 7);

                        std::string::size_type macroStart = start;
                        std::string::size_type macroElse = sourceString.find("#else", end);
                        std::string::size_type macroEnd = sourceString.find("#endif", end);
                        std::string::size_type nestedIfDef = sourceString.find("#ifdef", end);
                        NEO_ASSERT(macroEnd != std::string::npos, "Found an #ifdef %s without a matching #endif", define.c_str());
                        if (nestedIfDef != std::string::npos) {
                            NEO_ASSERT(nestedIfDef > macroEnd, "Can't have nested ifdefs");
                        }

                        if (defines.find(define) == defines.end()) {
                            if (macroElse != std::string::npos && macroElse < macroEnd) {
                                // remove endif
                                sourceString.erase(macroEnd, 6);
                                // remove else
                                sourceString.erase(macroElse, 5);
                                // remove ifdef block to else
                                sourceString.erase(macroStart, macroElse - macroStart);
                            }
                            else {
                                sourceString.erase(macroStart, macroEnd - macroStart + 6);
                            }
                            end = start;
                        }
                        else {
                            if (macroElse != std::string::npos && macroElse < macroEnd) {
                                // remove endif
                                sourceString.erase(macroEnd, 6);
                                // remove else block to endif
                                sourceString.erase(macroElse, macroEnd - macroElse);
                                // remove ifdef
                                sourceString.erase(macroStart, end - macroStart);
                            }
                            else {
                                // Erase endif
                                sourceString.erase(macroEnd, 6);
                                // Erase ifdef
                                sourceString.erase(macroStart, end - macroStart);
                            }
                            end = start;
                        }
                    }
                    start = end + 1;
                }
            }

            return sourceString;
        }

        static void _findUniforms(const char *shaderString, std::vector<std::string>& uniforms, std::map<std::string, GLint>& bindings) {
            std::string fileText(shaderString);
            std::string::size_type start = 0;
            std::string::size_type end = 0;
            while ((end = fileText.find("\n", start)) != std::string::npos) {
                std::string line = fileText.substr(start, end - start);

                std::string::size_type uniformStart = line.find("uniform");
                std::string::size_type bindingLoc = line.find("binding =");
                if (uniformStart != std::string::npos) {
                    std::string::size_type uniformTypeStart = line.find_first_not_of(' ', uniformStart + 7);
                    std::string::size_type uniformTypeEnd = line.find_first_of(' ', uniformTypeStart);
                    std::string::size_type uniformNameStart = line.find_first_not_of(' ', uniformTypeEnd);
                    std::string::size_type uniformNameEnd = line.find(";", uniformNameStart);
                    NEO_ASSERT(
                        uniformTypeStart != std::string::npos
                        && uniformTypeEnd != std::string::npos
                        && uniformNameStart != std::string::npos
                        && uniformNameEnd != std::string::npos, "Failed to parse uniform at line %s", line.c_str());
                    std::string uniform = line.substr(uniformNameStart, uniformNameEnd - uniformNameStart);
                    NEO_ASSERT(uniform.find(" ", uniformNameStart) == std::string::npos && uniform.find(",", uniformNameStart), "Can't have nested uniform definitions");
                    uniforms.push_back(uniform);

                    if (bindingLoc != std::string::npos) {
                        std::string::size_type bindingValueStart = line.find_first_not_of(' ', bindingLoc + 9);
                        std::string::size_type bindingValueEnd = line.find(")", bindingValueStart);
                        NEO_ASSERT(
                            bindingValueStart != std::string::npos
                            && bindingValueEnd != std::string::npos, "Filaed to parse binding at %s", line.c_str());
                       bindings.emplace(uniform, std::stoi(line.substr(bindingValueStart, bindingValueEnd - bindingValueStart)));
                    }
                }

                start = end + 1;
            }
        }
    }

    bool ResolvedShaderInstance::init(const NewShader::ShaderSources& args, const NewShader::ShaderDefines& defines) {
        NEO_ASSERT(!mValid && mPid == 0, "TODO");
        mValid = false;
        mPid = glCreateProgram();

        std::vector<std::string> uniforms;
        std::map<std::string, GLint> bindings;
        for (auto&& [stage, source] : args) {
            if (!source) {
                NEO_LOG_E("Trying to compile an empty shader source");
                return mValid;
            }
            std::string processedSource = _processShader(source, defines);
            if (processedSource.size()) {
                mShaderIDs[stage] = _compileShader(GLHelper::getGLShaderStage(stage), processedSource.c_str());
                if (mShaderIDs[stage]) {
                    glAttachShader(mPid, mShaderIDs[stage]);
                    _findUniforms(processedSource.c_str(), uniforms, bindings);
                }
                else {
                    mValid = false;
                    return mValid;
                }
            }
        }
        glLinkProgram(mPid);

        // See whether link was successful
        GLint linkSuccess;
        glGetProgramiv(mPid, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess) {
            GLHelper::printProgramInfoLog(mPid);
            destroy();
            mValid = false;
            return mValid;
        }

       mValid = true;
       for (auto& uniform : uniforms) {
           GLint r = glGetUniformLocation(mPid, uniform.c_str());
           if (r < 0) {
               NEO_LOG_S(util::LogSeverity::Warning, "WARN: %s uniform cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it", uniform.c_str());
           }
           mUniforms[HashedString(uniform.c_str()).value()] = r;
       }
       for (auto& binding : bindings) {
           mBindings[HashedString(binding.first.c_str()).value()] = binding.second;
       }

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
            glDeleteShader(shader);
            return 0;
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
        glActiveTexture(GL_TEXTURE0);
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
        GLint bindingLoc = 0;
        auto binding = mBindings.find(HashedString(name));
        if (binding != mBindings.end()) {
            bindingLoc = binding->second;
        }
        glActiveTexture(GL_TEXTURE0 + bindingLoc);
        texture.bind();
        glUniform1i(_getUniform(name), texture.mTextureID);

    }
}