#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Loader/Loader.hpp"
#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public PostProcessShader {

    public:

        bool showAO = true;
        float diffuseAmount = 0.2f;

        CombineShader(const std::string &frag) :
            PostProcessShader ("Combine Shader", frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            loadUniform("showAO", showAO);
            loadUniform("diffuseAmount", diffuseAmount);

            // Bind diffuse output
            auto gBuffer = Loader::getFBO("gbuffer");
            gBuffer->mTextures[1]->bind();
            loadUniform("gDiffuse", gBuffer->mTextures[1]->mTextureID);

            // Bind light pass output
            auto lightFBO = Loader::getFBO("lightpass");
            lightFBO->mTextures[0]->bind();
            loadUniform("lightOutput", lightFBO->mTextures[0]->mTextureID);
        }
};
