#pragma once

#include "Component/RenderableComponent/CubeMapComponent.hpp"
#include "Shader/Shader.hpp"

#include "util/GLHelper.hpp"

using namespace neo;

class SkyboxShader : public Shader {

    public:

        SkyboxShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader("Skybox Shader", res, vert, frag)
        {}

        virtual void render(float dt, const RenderSystem &renderSystem) override;
};