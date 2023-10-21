#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/NewShader.hpp"

#include <glm/glm.hpp>

#include <string>

namespace neo {

    struct ShaderComponent : public Component {
        ShaderComponent(NewShader* shader);

        void bind() const;
        void bindUniform(const char* name, NewShader::UniformVariant uniform) const;
        void bindTexture(const char* name, Texture& texture) const;

        virtual std::string getName() const override {
            return "ShaderComponent";
        }

        virtual void imGuiEditor() override {
        }

    private:
        NewShader* mShaderSource;


    };
}