#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/NewShader.hpp"

#include <glm/glm.hpp>

#include <string>

namespace neo {

    struct ShaderComponent : public Component {
    public:
        ShaderComponent(NewShader* shader) : mShaderSource(shader) {}

        const ResolvedShaderInstance& getResolvedInstance(const NewShader::ShaderDefines& defines) const {
            MICROPROFILE_SCOPEI(getName().c_str(), "getResolvedInstance", MP_AUTO);
            return mShaderSource->getResolvedInstance(defines);
        }

        virtual std::string getName() const = 0;

    protected:
        NewShader* mShaderSource;
    };
}