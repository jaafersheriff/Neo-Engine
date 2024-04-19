#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"

#include <glm/glm.hpp>
#include <entt/container/dense_hash_map.hpp>

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
		bool isValid() const { return mValid; }

		void bind() const;
		void unbind() const;
		const std::string& variant() const { return mVariant; }

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
		uint32_t mPid = 0;
		entt::dense_hash_map<types::shader::Stage, uint32_t> mShaderIDs;
		entt::dense_hash_map<HashedString::hash_type, int32_t> mUniforms;
		entt::dense_hash_map<HashedString::hash_type, int32_t> mBindings;
		std::string mVariant;

		uint32_t _compileShader(uint32_t shaderType, const char* shaderString);
		int32_t _getUniform(const char* name) const;
	};
}