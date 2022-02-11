#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public PostProcessShader {

    public:

        float diffuseAmount = 0.2f;

        CombineShader(const std::string &frag) :
            PostProcessShader("Combine Shader", frag) 
        {}

        virtual void render() override {
            loadUniform("diffuseAmount", diffuseAmount);

            // Bind diffuse output
            loadTexture("gDiffuse", *Library::getFBO("gbuffer")->mTextures[1]);

            // Bind light pass output
            loadTexture("lightOutput", *Library::getFBO("lightpass")->mTextures[0]);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Global diffuse", &diffuseAmount, 0.f, 1.f);
        }
};
