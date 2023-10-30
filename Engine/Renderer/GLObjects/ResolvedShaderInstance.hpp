#pragma once

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
        using ShaderDefines = std::set<std::string>;
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

        void bind() const;
        void unbind() const;
        void bindUniform(const char* name, const UniformVariant& uniform) const;
        void bindTexture(const char* name, const Texture& texture) const;

    private:
        GLuint mPid = 0;
    };
}