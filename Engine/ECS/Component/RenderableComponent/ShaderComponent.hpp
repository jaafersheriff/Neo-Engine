#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/NewShader.hpp"

#include <glm/glm.hpp>

#include <unordered_map>
#include <string>
#include <variant>

namespace neo {
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
        glm::mat4,
        Texture
        >;

    struct ShaderComponent : public Component {
        ShaderComponent(NewShader* shader)
        {}

        void bind() const;
        void bindUniform(const char* name, UniformVariant uniform) const;

        virtual std::string getName() const override {
            return "ShaderComponent";
        }

        virtual void imGuiEditor() override {
        }

    private:
        std::unordered_map<std::string, UniformVariant> mUniforms;


    };
}