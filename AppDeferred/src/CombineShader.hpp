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

        CombineShader(const std::string &frag) :
            Shader("Combine Shader", MasterRenderer::POST_PROCESS_VERT_FILE, frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            auto lightFBO = Loader::getFBO("lightpass");
            lightFBO->textures[0]->bind();
            loadUniform("lightOutput", lightFBO->textures[0]->textureId);
            auto aoFBO = Loader::getFBO("AO");
            aoFBO->textures[0]->bind();
            loadUniform("aoOutput", aoFBO->textures[0]->textureId);
        }
};
