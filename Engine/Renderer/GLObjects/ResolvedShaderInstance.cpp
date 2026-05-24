#include "Renderer/pch.hpp"

#include "ResolvedShaderInstance.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/ShaderBuffer.hpp"
#include "Renderer/GLObjects/ShaderBarrier.hpp"

#include "Util/Util.hpp"
#include "Loader/Loader.hpp"

namespace neo {
	namespace {
		#define GET_FILE_LINE (std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str()

		void printShaderInfoLog(GLuint shader) {
			GLint infologLength = 0;
			GLint charsWritten = 0;
			GLchar *infoLog;

			GLHelper::checkError(GET_FILE_LINE);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
			GLHelper::checkError(GET_FILE_LINE);

			if (infologLength > 0) {
				infoLog = new GLchar[infologLength + 1];
				glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
				infoLog[charsWritten + 1] = '\0';
				NEO_LOG_E("Shader InfoLog:\n%s", infoLog);
				delete[] infoLog;
			}
			GLHelper::checkError(GET_FILE_LINE);
		}

		void printProgramInfoLog(GLuint program) {
			GLint infologLength = 0;
			GLint charsWritten = 0;
			GLchar *infoLog;

			GLHelper::checkError(GET_FILE_LINE);
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
			GLHelper::checkError(GET_FILE_LINE);

			if (infologLength > 0) {
				infoLog = new GLchar[infologLength];
				NEO_ASSERT(infoLog != NULL, "Could not allocate InfoLog buffer");
				glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
				NEO_LOG_E("Program InfoLog:\n%s\n\n", infoLog);
				delete[] infoLog;
			}
			GLHelper::checkError(GET_FILE_LINE);
		}

		int32_t _getGLAccessType(types::shader::Access accessType) {
			switch (accessType) {
			case types::shader::Access::Read:
				return GL_READ_ONLY;
			case types::shader::Access::Write:
				return GL_WRITE_ONLY;
			case types::shader::Access::ReadWrite:
				return GL_READ_WRITE;
			default:
				NEO_FAIL("Invalid access type");
				return 0;
			}
		}

		int32_t _getGLShaderStage(types::shader::Stage type) {
			switch (type) {
			case(types::shader::Stage::Vertex):
				return GL_VERTEX_SHADER;
			case(types::shader::Stage::Fragment):
				return GL_FRAGMENT_SHADER;
			case(types::shader::Stage::Geometry):
				return GL_GEOMETRY_SHADER;
			case(types::shader::Stage::Compute):
				return GL_COMPUTE_SHADER;
			case(types::shader::Stage::TessellationControl):
				return GL_TESS_CONTROL_SHADER;
			case(types::shader::Stage::TessellationEval):
				return GL_TESS_EVALUATION_SHADER;
			default:
				NEO_FAIL("Invalid Shadertypes::shader::Stage: %d", type);
				return 0;
			}
		}

		inline std::string _processShader(const char* shaderString, const ShaderDefines& defines) {
			TRACY_ZONE();
			if (!shaderString) {
				return "";
			}
			std::string sourceString(shaderString);
			sourceString = "#include \"universal.glsl\"\n" + sourceString;

			// Handle #includes 
			{
				TRACY_ZONEN("Handle includes");
				std::string::size_type start = 0;
				std::string::size_type end = 0;
				while ((end = sourceString.find("\n", start)) != std::string::npos) {
					std::string line = sourceString.substr(start, end - start);

					// Find name
					std::string::size_type nameStart = line.find("#include \"");
					std::string::size_type nameEnd = line.find("\"", nameStart + 10);
					if (nameStart != std::string::npos && nameEnd != std::string::npos && nameStart != nameEnd) {
						std::string includedFile = line.substr(nameStart + 10, nameEnd - nameStart - 10);
						const char* includedFileSrc = Loader::loadFileString(includedFile.c_str());

						sourceString.erase(start, end - start);
						if (includedFileSrc) {
							// Replace include with source
							sourceString.insert(start, includedFileSrc);
							delete includedFileSrc;
						}
						else {
							NEO_LOG_E("Found #include %s but it's empty? Skipping", includedFile.c_str());
						}

						// Reset processing incase there are internal #includes
						start = end = 0;

					}
					start = end + 1;
				}
			}

			// #version, #defines
			std::stringstream preambleBuilder;
			{
				TRACY_ZONEN("Construct preamble");
				preambleBuilder << ServiceLocator<Renderer>::ref().getDetails().mGLSLVersion << "\n\n";
				const ShaderDefines* _defines = &defines;
				while (_defines) {
					for (auto& define : _defines->mDefines) {
						if (define.second) {
							preambleBuilder << "#define " << define.first.mVal.data() << "\n";
						}

					}
					_defines = _defines->mParent;
				}
				sourceString.insert(0, preambleBuilder.str());
			}

			return sourceString;
		}

		inline void _findBinding(const std::string& line, const std::string& uniformName, std::map<std::string, GLint>& bindings) {
			std::string::size_type bindingLoc = line.find("binding =");
			if (bindingLoc != std::string::npos) {
				std::string::size_type bindingValueStart = line.find_first_not_of(' ', bindingLoc + 9);
				std::string::size_type bindingValueEnd = std::string::npos;
				std::string::size_type bindingValueEndComma = line.find(",", bindingValueStart);
				std::string::size_type bindingValueEndParen = line.find(")", bindingValueStart);
				if (bindingValueEndComma != std::string::npos && bindingValueEndParen != std::string::npos) {
					bindingValueEnd = bindingValueEndComma < bindingValueEndParen ? bindingValueEndComma : bindingValueEndParen;
				}
				else {
					bindingValueEnd = bindingValueEndParen;
				}
				NEO_ASSERT(
					bindingValueStart != std::string::npos
					&& bindingValueEnd != std::string::npos, "Failed to parse binding at %s", line.c_str());
				bindings.emplace(uniformName, std::stoi(line.substr(bindingValueStart, bindingValueEnd - bindingValueStart)));
			}
		}

		inline void _findUniforms(const char *shaderString, std::vector<std::string>& uniforms, std::map<std::string, GLint>& bindings) {
			TRACY_ZONE();
			std::string fileText(shaderString);
			std::string::size_type start = 0;
			std::string::size_type end = 0;
			while ((end = fileText.find("\n", start)) != std::string::npos) {
				std::string line = fileText.substr(start, end - start);
				std::string::size_type commentStart = line.find("//");
				if (commentStart == 0) {
					start = end + 1;
					continue;
				}

				// Handle layout syntax
				std::string::size_type layoutStart = line.find("layout");
				if (layoutStart != std::string::npos && layoutStart < commentStart) {
					std::string::size_type bindingLoc = line.find("binding =", layoutStart);
					if (bindingLoc != std::string::npos) {
						std::string::size_type bindingValueStart = line.find_first_not_of(' ', bindingLoc + 9);
						std::string::size_type bindingValueEnd = line.find_first_of(",)", bindingValueStart);
						GLint bindingValue = std::stoi(line.substr(bindingValueStart, bindingValueEnd - bindingValueStart));

						std::string::size_type uniformStart = line.find("uniform", bindingValueEnd);
						if (uniformStart != std::string::npos) {
							std::string::size_type uniformNameStart = line.find_first_not_of(' ', uniformStart + 7);
							std::string::size_type uniformNameEnd = line.find_first_of(' ', uniformNameStart);
							std::string uniform = line.substr(uniformNameStart, uniformNameEnd - uniformNameStart);

							uniforms.push_back(uniform);
							bindings.emplace(uniform, bindingValue);
						}
					}
				}

				// Handle old uniform syntax
				std::string::size_type uniformStart = line.find("uniform");
				if (uniformStart != std::string::npos && (commentStart == std::string::npos || uniformStart < commentStart)) {
                    std::string::size_type uniformTypeStart = line.find_first_not_of(' ', uniformStart + 7);
                    if (uniformTypeStart == std::string::npos) {
                        start = end + 1;
                        continue; // Skip if no type is found
                    }

                    std::string::size_type uniformTypeEnd = line.find_first_of(' ', uniformTypeStart);
                    if (uniformTypeEnd == std::string::npos) {
                        start = end + 1;
                        continue; // Skip if no type end is found
                    }

                    std::string::size_type uniformNameStart = line.find_first_not_of(' ', uniformTypeEnd);
                    if (uniformNameStart == std::string::npos) {
                        start = end + 1;
                        continue; // Skip if no name start is found
                    }

                    std::string::size_type uniformNameEnd = line.find(";", uniformNameStart);
                    if (uniformNameEnd == std::string::npos) {
                        start = end + 1;
                        continue; // Skip if no semicolon is found
                    }

                    std::string uniform = line.substr(uniformNameStart, uniformNameEnd - uniformNameStart);
                    if (uniform.find(" ") != std::string::npos || uniform.find(",") != std::string::npos) {
                        start = end + 1;
                        continue; // Skip if the uniform name is malformed
                    }

                    uniforms.push_back(uniform);
                    _findBinding(line, uniform, bindings);
                }

				// Handle buffer syntax
				std::string::size_type bufferStart = line.find("buffer");
				if (bufferStart != std::string::npos && bufferStart < commentStart) {
					std::string::size_type bufferNameStart = line.find_first_not_of(' ', bufferStart + 6);
					std::string::size_type bufferNameEnd = line.find_first_of(' ', bufferNameStart);
					if (bufferNameEnd == std::string::npos) {
						bufferNameEnd = line.find_first_of('\n', bufferNameStart);
					}
					NEO_ASSERT(
						bufferNameStart != std::string::npos
						&& bufferNameEnd != std::string::npos, "Failed to parse buffer at line %s", line.c_str());
					std::string buffer = line.substr(bufferNameStart, bufferNameEnd - bufferNameStart);
					NEO_ASSERT(buffer.find(" ") == std::string::npos && buffer.find(",") == std::string::npos, "Can't have nested uniform definitions at %s", line.c_str());
					uniforms.push_back(buffer);

					_findBinding(line, buffer, bindings);
				}

				start = end + 1;
			}
		}
	}

	bool ResolvedShaderInstance::init(const SourceShader::ShaderCode& shaderCode, const ShaderDefines& defines) {
		NEO_ASSERT(!isValid() && mPid == 0, "Trying to initialize an existing shader variant object?");
		TRACY_ZONE();
		isCompute = false;
		mPid = glCreateProgram();

		std::vector<std::string> uniforms;
		std::map<std::string, GLint> bindings;
		for (auto&& [stage, source] : shaderCode) {
			if (stage == types::shader::Stage::Compute) {
				isCompute = true;
			}
			if (!source) {
				NEO_LOG_E("Trying to compile an empty shader source");
				glDeleteProgram(mPid);
				mPid = 0;
				return false;
			}
			std::string processedSource = _processShader(source, defines);
			if (processedSource.size()) {
				mShaderIDs[stage] = _compileShader(_getGLShaderStage(stage), processedSource.c_str());
				if (mShaderIDs[stage]) {
					glAttachShader(mPid, mShaderIDs[stage]);
					_findUniforms(processedSource.c_str(), uniforms, bindings);
				}
				else {
					glDeleteProgram(mPid);
					mPid = 0;
					return false;
				}
			}
		}
		glLinkProgram(mPid);

		// See whether link was successful
		GLint linkSuccess;
		glGetProgramiv(mPid, GL_LINK_STATUS, &linkSuccess);
		if (!linkSuccess) {
			printProgramInfoLog(mPid);
			return false;
		}

	   // This might break if different shader stages use the same uniform..?
	   for (auto& uniform : uniforms) {
		   GLint r = glGetUniformLocation(mPid, uniform.c_str());
		   if (r >= 0) {
			   mUniforms[HashedString(uniform.c_str()).value()] = r;
		   }
	   }
	   for (auto& binding : bindings) {
		   mBindings[HashedString(binding.first.c_str()).value()] = binding.second;
	   }

		return true;
	}

	uint32_t ResolvedShaderInstance::_compileShader(GLenum shaderType, const char *shaderString) {
		TRACY_ZONE();
		// Create the shader, assign source code, and compile it
		GLuint shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &shaderString, NULL);
		glCompileShader(shader);

		// See whether compile was successful
		GLint compileSuccess;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
		if (!compileSuccess) {
			printShaderInfoLog(shader);
			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	void ResolvedShaderInstance::destroy() {
		if (mPid) {
			unbind();
			for (auto&& [stage, id] : mShaderIDs) {
				if (id > 0) {
					glDetachShader(mPid, id);
					glDeleteShader(id);
				}
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

	void ResolvedShaderInstance::dispatch(glm::uvec3 workGroups) const {
		if (isCompute) {
			glDispatchCompute(workGroups.x, workGroups.y, workGroups.z);
		}
		else {
			//NEO_LOG_E("Trying to dispatch non-compute shader");
		}
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
		util::visit(uniform, 
			[&](bool b) { glUniform1i(_getUniform(name), b); },
			[&](int i) { glUniform1i(_getUniform(name), i); },
			[&](uint16_t i) { glUniform1ui(_getUniform(name), i); },
			[&](uint32_t i) { glUniform1ui(_getUniform(name), i); },
			[&](double d) { glUniform1f(_getUniform(name), static_cast<float>(d)); },
			[&](float f) { glUniform1f(_getUniform(name), static_cast<float>(f)); },
			[&](glm::vec2 v) { glUniform2f(_getUniform(name), v.x, v.y); },
			[&](glm::ivec2 v) { glUniform2i(_getUniform(name), v.x, v.y); },
			[&](glm::uvec2 v) { glUniform2ui(_getUniform(name), v.x, v.y); },
			[&](glm::vec3 v) { glUniform3f(_getUniform(name), v.x, v.y, v.z); },
			[&](glm::ivec3 v) { glUniform3i(_getUniform(name), v.x, v.y, v.z); },
			[&](glm::uvec3 v) { glUniform3ui(_getUniform(name), v.x, v.y, v.z); },
			[&](glm::vec4 v) { glUniform4f(_getUniform(name), v.x, v.y, v.z, v.w); },
			[&](glm::mat3 m) { glUniformMatrix3fv(_getUniform(name), 1, GL_FALSE, &m[0][0]); },
			[&](glm::mat4 m) { glUniformMatrix4fv(_getUniform(name), 1, GL_FALSE, &m[0][0]); },
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
		);
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
		glUniform1i(_getUniform(name), bindingLoc);
	}

	void ResolvedShaderInstance::bindImageTexture(const char* name, const Texture& texture, types::shader::Access accessType, int mip) const {
		GLint bindingLoc = 0;
		auto binding = mBindings.find(HashedString(name));
		if (binding != mBindings.end()) {
			bindingLoc = binding->second;
		}
		glBindImageTexture(bindingLoc, texture.mTextureID, mip, GL_FALSE, 0, _getGLAccessType(accessType), GLHelper::getGLInternalFormat(texture.mFormat.mInternalFormat));
		// return ShaderBarrier(accessType > types::shader::Access::Read ? types::shader::Barrier::ImageAccess : types::shader::Barrier::None); // I'm really trusting the compiler to use copy elision here
	}

	void ResolvedShaderInstance::bindShaderBuffer(const char* name, const ShaderBuffer& buffer, types::shader::Access accessType) const {
		if (buffer.mTarget == types::buffer::Target::ShaderStorage) {
			return _bindShaderBuffer(name, GL_SHADER_STORAGE_BUFFER, buffer.mBufferID, accessType);
		}
		else {
			return _bindShaderBuffer(name, GL_UNIFORM_BUFFER, buffer.mBufferID, accessType);
		}
	}

	void ResolvedShaderInstance::bindShaderBuffer(const char* name, const Mesh& mesh, types::mesh::VertexType vertexType, types::shader::Access accessType) const {
		return _bindShaderBuffer(name, GL_SHADER_STORAGE_BUFFER, mesh.getVBO(vertexType).vboID, accessType);
	}

	void ResolvedShaderInstance::bindMeshIndices(const char* name, const Mesh& mesh, types::shader::Access accessType) const {
		NEO_ASSERT(mesh.hasIBO(), "Trying to bind mesh indices but mesh doesn't have an IBO");
		return _bindShaderBuffer(name, GL_SHADER_STORAGE_BUFFER, mesh.getIBO().vboID, accessType);
	}

	void ResolvedShaderInstance::_bindShaderBuffer(const char* name, unsigned int glTarget, uint32_t id, types::shader::Access accessType) const {
		NEO_UNUSED(accessType);
		GLint bindingLoc = 0;
		auto binding = mBindings.find(HashedString(name));
		if (binding != mBindings.end()) {
			bindingLoc = binding->second;
		}
		glBindBufferBase(glTarget, bindingLoc, id);
		// return ShaderBarrier(accessType > types::shader::Access::Read ? types::shader::Barrier::StorageBuffer : types::shader::Barrier::None); // I'm really trusting the compiler to use copy elision here
	}

}
