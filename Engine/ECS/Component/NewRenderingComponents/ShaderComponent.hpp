#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/NewShader.hpp"

#include <glm/glm.hpp>

#include <string>

namespace neo {

    struct ShaderComponent : public Component {
        ShaderComponent(NewShader* shader);

        ResolvedShaderInstance getResolvedInstance(const NewShader::ShaderDefines& defines) const;

        virtual std::string getName() const = 0;

    protected:
        NewShader* mShaderSource;
    };
}