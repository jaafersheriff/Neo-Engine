#pragma once

#include "Renderer/GLObjects/NewShader.hpp"

#include <glm/glm.hpp>
#include <gl/glew.h>

#include <variant>
#include <set>
#include <string>

namespace neo {
    class Texture;
    class NewShader;


    class ResolvedShaderInstance {
        friend NewShader;
    public:

        ResolvedShaderInstance() = default;
        ~ResolvedShaderInstance() = default;

        bool init(const NewShader::ShaderSources& args, const NewShader::ShaderDefines& defines);
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

        GLuint _compileShader(GLenum shaderType, const char* shaderString);
        GLint _getUniform(const char* name) const;
    };
}