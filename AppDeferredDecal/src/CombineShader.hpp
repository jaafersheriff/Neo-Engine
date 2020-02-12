#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
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
            auto gBuffer = Library::getFBO("gbuffer");
            loadTexture("gDiffuse", *gBuffer->mTextures[1]);

            // Bind light pass output
            auto lightFBO = Library::getFBO("lightpass");
            loadTexture("lightOutput", *lightFBO->mTextures[0]);

            // Decals
            auto decalFBO = Library::getFBO("decals");
            loadTexture("decals", *decalFBO->mTextures[0]);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Global diffuse", &diffuseAmount, 0.f, 1.f);
        }

};
