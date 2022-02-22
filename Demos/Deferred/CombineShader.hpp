#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

namespace Deferred {
    class CombineShader : public PostProcessShader {

    public:

        bool showAO = true;
        float diffuseAmount = 0.1f;

        CombineShader(const std::string& frag) :
            PostProcessShader("Combine Shader", frag)
        {}

        virtual void render(const ECS& ecs) override {
            NEO_UNUSED(ecs);
            loadUniform("showAO", showAO);
            loadUniform("diffuseAmount", diffuseAmount);

            // Bind diffuse output
            loadTexture("gDiffuse", *Library::getFBO("gbuffer")->mTextures[1]);

            // Bind light pass output
            loadTexture("lightOutput", *Library::getFBO("lightpass")->mTextures[0]);

            // Decals
            auto decalFBO = Library::getFBO("decals");
            loadTexture("decals", *decalFBO->mTextures[0]);
        }

        virtual void imguiEditor() override {
            ImGui::Checkbox("Show AO", &showAO);
            ImGui::SliderFloat("Diffuse", &diffuseAmount, 0.f, 1.f);
        }
    };
}
