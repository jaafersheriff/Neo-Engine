#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"

#include <glm/glm.hpp>

#include <string>

namespace neo {

    struct ShaderComponent : public Component {
    public:
        ShaderComponent(SourceShader* shader) : mSourceShader(shader) {}

        const ResolvedShaderInstance& getResolvedInstance(const ShaderDefines& defines) const {
            return mSourceShader->getResolvedInstance(defines);
        }

        virtual std::string getName() const = 0;

    protected:
        SourceShader* mSourceShader;
    };
}