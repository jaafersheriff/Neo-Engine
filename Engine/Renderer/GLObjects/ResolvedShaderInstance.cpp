#include "Renderer/pch.hpp"

#include "ResolvedShaderInstance.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "Util/Util.hpp"
#include "Loader/Loader.hpp"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

namespace neo {
	namespace {


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
		static std::string _processShader(const char* shaderString, const ShaderDefines& defines) {
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
				preambleBuilder << ServiceLocator<Renderer>::ref().mDetails.mGLSLVersion << "\n";
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

			// Use glslang just for the preprocessor..for now
			// Maybe spirv and validators and stuff can happen later
			{
				TRACY_ZONEN("glslang");
				NEO_ASSERT(glslang_initialize_process(), "Failed to initialize glslang");

				glslang_input_t input;
				input.language = GLSLANG_SOURCE_GLSL;
				input.stage = GLSLANG_STAGE_VERTEX;
				input.client = GLSLANG_CLIENT_OPENGL;
				input.client_version = GLSLANG_TARGET_OPENGL_450;
				input.target_language = GLSLANG_TARGET_NONE;
				// input.target_language_version;
				input.code = sourceString.c_str();
				input.default_version = 430;
				input.default_profile = GLSLANG_NO_PROFILE;
				input.force_default_version_and_profile = false;
				input.forward_compatible = false;
				input.messages = GLSLANG_MSG_ONLY_PREPROCESSOR_BIT;
				input.resource = glslang_default_resource();

				glslang_shader_t* shader = glslang_shader_create(&input);
				NEO_ASSERT(shader, "glslang_shader_create failed");

				glslang_shader_preprocess(shader, &input);

				std::string ret = glslang_shader_get_preprocessed_code(shader);

				glslang_finalize_process();

				return ret;
			}
		}

		static void _findUniforms(const char *shaderString, std::vector<std::string>& uniforms, std::map<std::string, GLint>& bindings) {
			TRACY_ZONE();
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
					NEO_ASSERT(uniform.find(" ") == std::string::npos && uniform.find(",") == std::string::npos, "Can't have nested uniform definitions at %s", line.c_str());
					uniforms.push_back(uniform);

					if (bindingLoc != std::string::npos) {
						std::string::size_type bindingValueStart = line.find_first_not_of(' ', bindingLoc + 9);
						std::string::size_type bindingValueEnd = line.find(")", bindingValueStart);
						NEO_ASSERT(
							bindingValueStart != std::string::npos
							&& bindingValueEnd != std::string::npos, "Failed to parse binding at %s", line.c_str());
					   bindings.emplace(uniform, std::stoi(line.substr(bindingValueStart, bindingValueEnd - bindingValueStart)));
					}
				}

				start = end + 1;
			}
		}
	}

	bool ResolvedShaderInstance::init(const SourceShader::ShaderCode& shaderCode, const ShaderDefines& defines) {
		NEO_ASSERT(!mValid && mPid == 0, "Trying to initialize an existing shader variant object?");
		TRACY_ZONE();
		mValid = false;
		mPid = glCreateProgram();

		std::vector<std::string> uniforms;
		std::map<std::string, GLint> bindings;
		for (auto&& [stage, source] : shaderCode) {
			if (!source) {
				NEO_LOG_E("Trying to compile an empty shader source");
				return mValid;
			}
			std::string processedSource = _processShader(source, defines);
			if (processedSource.size()) {
				mShaderIDs[stage] = _compileShader(_getGLShaderStage(stage), processedSource.c_str());
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
	   // This might break if different shader stages use the same uniform..?
	   for (auto& uniform : uniforms) {
		   GLint r = glGetUniformLocation(mPid, uniform.c_str());
		   if (r < 0) {
			   NEO_LOG_S(util::LogSeverity::Warning, "%s uniform cannot be bound (it either doesn't exist or has been optimized away)", uniform.c_str());
		   }
		   mUniforms[HashedString(uniform.c_str()).value()] = r;
	   }
	   for (auto& binding : bindings) {
		   mBindings[HashedString(binding.first.c_str()).value()] = binding.second;
	   }

		return mValid;
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
		std::visit(util::VisitOverloaded{
			[&](bool b) { glUniform1i(_getUniform(name), b); },
			[&](int i) { glUniform1i(_getUniform(name), i); },
			[&](uint32_t i) { glUniform1ui(_getUniform(name), i); },
			[&](double d) { glUniform1f(_getUniform(name), static_cast<float>(d)); },
			[&](float f) { glUniform1f(_getUniform(name), static_cast<float>(f)); },
			[&](glm::vec2 v) { glUniform2f(_getUniform(name), v.x, v.y); },
			[&](glm::ivec2 v) { glUniform2i(_getUniform(name), v.x, v.y); },
			[&](glm::uvec2 v) { glUniform2i(_getUniform(name), v.x, v.y); },
			[&](glm::vec3 v) { glUniform3f(_getUniform(name), v.x, v.y, v.z); },
			[&](glm::vec4 v) { glUniform4f(_getUniform(name), v.x, v.y, v.z, v.w); },
			[&](glm::mat3 m) { glUniformMatrix3fv(_getUniform(name), 1, GL_FALSE, &m[0][0]); },
			[&](glm::mat4 m) { glUniformMatrix4fv(_getUniform(name), 1, GL_FALSE, &m[0][0]); },
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
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
		glUniform1i(_getUniform(name), bindingLoc);
	}
}