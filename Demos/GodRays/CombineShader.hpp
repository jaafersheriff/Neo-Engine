#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

namespace GodRays {
    class CombineShader : public PostProcessShader {

    public:

        float exposure = 0.9f;

        CombineShader(const std::string& frag) :
            PostProcessShader("Combine Shader", frag)
        {}

        virtual void render(const ECS& ecs) override {
            loadTexture("godray", *Library::getFBO("godrayblur")->mTextures[0]);

            loadUniform("sunColor", ecs.getSingleComponent<SunComponent>()->getGameObject().getComponentByType<LightComponent>()->mColor);
            loadUniform("exposure", exposure);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Exposure", &exposure, 0.01f, 10.f);
        }
    };
}
