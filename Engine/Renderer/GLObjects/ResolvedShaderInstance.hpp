#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"

#include <glm/glm.hpp>
#include <gl/glew.h>

#include <variant>
#include <set>
#include <string>

namespace neo {
	class Texture;
	class SourceShader;

	class ResolvedShaderInstance {
		friend SourceShader;
	public:

		ResolvedShaderInstance(std::string variant) : mVariant(variant) {}
		~ResolvedShaderInstance() = default;

		bool init(const SourceShader::ShaderCode& args, const ShaderDefines& defines);
		void destroy();

		void bind() const;
		void unbind() const;

		using UniformVariant =
			std::variant<
			bool,
			int,
			uint32_t,
			double,
			float,
			glm::vec2,
			glm::ivec2,
			glm::uvec2,
			glm::vec3,
			glm::vec4,
			glm::mat3,
			glm::mat4
			>;
		void bindUniform(const char* name, const UniformVariant& uniform) const;
		void bindTexture(const char* name, const Texture& texture) const;

	private:
		bool mValid = false;
		GLuint mPid = 0;
		std::unordered_map<ShaderStage, GLuint> mShaderIDs;
		std::unordered_map<HashedString::hash_type, GLint> mUniforms;
		std::unordered_map<HashedString::hash_type, GLint> mBindings;
		std::string mVariant;

		GLuint _compileShader(GLenum shaderType, const char* shaderString);
		GLint _getUniform(const char* name) const;
	};
}