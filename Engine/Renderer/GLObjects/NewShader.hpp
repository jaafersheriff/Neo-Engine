#pragma once

// This should just be 
// Name
// Shader file path
// Shader source
// map<ShaderDefines, PID>

#include <variant>

namespace neo {
	class NewShader {
    public:
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


	};
}