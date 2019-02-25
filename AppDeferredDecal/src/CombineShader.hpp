#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"
#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public Shader {

    public:

        float diffuseAmount = 0.2f;

        CombineShader(const std::string &frag) :
            Shader("Combine Shader", MasterRenderer::POST_PROCESS_VERT_FILE, frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            loadUniform("diffuseAmount", diffuseAmount);

            // Bind diffuse output
            auto gBuffer = Loader::getFBO("gbuffer");
            gBuffer->mTextures[1]->bind();
            loadUniform("gDiffuse", gBuffer->mTextures[1]->mTextureID);

            // Bind light pass output
            auto lightFBO = Loader::getFBO("lightpass");
            lightFBO->mTextures[0]->bind();
            loadUniform("lightOutput", lightFBO->mTextures[0]->mTextureID);

            // Decals
            auto decalFBO = Loader::getFBO("decals");
            decalFBO->mTextures[0]->bind();
            loadUniform("decals", decalFBO->mTextures[0]->mTextureID);
        }
};
