#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

class CombineShader : public neo::PostProcessShader {

    public:

        float diffuseAmount = 0.2f;

        CombineShader(const std::string &frag) :
            neo::PostProcessShader("Combine Shader", frag) 
        {}

        virtual void render(const neo::CameraComponent &camera) override {
            loadUniform("diffuseAmount", diffuseAmount);

            // Bind diffuse output
            auto gBuffer = neo::Library::getFBO("gbuffer");
            gBuffer->mTextures[1]->bind();
            loadUniform("gDiffuse", gBuffer->mTextures[1]->mTextureID);

            // Bind light pass output
            auto lightFBO = neo::Library::getFBO("lightpass");
            lightFBO->mTextures[0]->bind();
            loadUniform("lightOutput", lightFBO->mTextures[0]->mTextureID);
        }
};
