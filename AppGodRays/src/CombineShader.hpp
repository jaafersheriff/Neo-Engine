#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public PostProcessShader {

    public:

        CombineShader(const std::string &frag) :
            PostProcessShader("Combine Shader", frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            auto godray = Library::getFBO("godrayblur");
            godray->mTextures[0]->bind();
            loadUniform("godray", godray->mTextures[0]->mTextureID);
        }

        virtual void imguiEditor() override {
        }
};
