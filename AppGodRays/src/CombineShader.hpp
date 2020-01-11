#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public PostProcessShader {

    public:

        float exposure = 0.9f;

        CombineShader(const std::string &frag) :
            PostProcessShader("Combine Shader", frag) 
        {}

        virtual void render() override {
            loadTexture("godray", *Library::getFBO("godrayblur")->mTextures[0]);

            loadUniform("sunColor", Engine::getSingleComponent<SunComponent>()->getGameObject().getComponentByType<LightComponent>()->mColor);
            loadUniform("exposure", exposure);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Exposure", &exposure, 0.01f, 10.f);
        }
};
