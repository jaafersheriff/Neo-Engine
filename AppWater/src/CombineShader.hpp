#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public PostProcessShader {

    public:

        float diffuseAmount = 0.2f;

        CombineShader(const std::string &frag) :
            PostProcessShader("Combine Shader", frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            loadUniform("diffuseAmount", diffuseAmount);

            // Bind diffuse output
            auto gBuffer = Library::getFBO("gbuffer");
            loadTexture("gDiffuse", *gBuffer->mTextures[2]);
            loadTexture("gDepth", *gBuffer->mTextures[3]);

            // Bind light pass output
            auto lightFBO = Library::getFBO("lightpass");
            lightFBO->mTextures[0]->bind();
            loadUniform("lightOutput", lightFBO->mTextures[0]->mTextureID);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Global diffuse", &diffuseAmount, 0.f, 1.f);
        }
};
