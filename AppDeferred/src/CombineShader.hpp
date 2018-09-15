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
        {
            // Create render target
            auto combineFBO = Loader::getFBO("combine");
            combineFBO->generate();
            combineFBO->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT); // color

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                auto combineFBO = Loader::getFBO("combine");
                combineFBO->textures[0]->width  = frameSize.x;
                combineFBO->textures[0]->height = frameSize.y;
                combineFBO->textures[0]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameSize.x, frameSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
            });
        }

        virtual void render(const CameraComponent &camera) override {
            auto lightFBO = Loader::getFBO("lightpass");
            lightFBO->textures[0]->bind();
            loadUniform("lightOutput", lightFBO->textures[0]->textureId);
            auto aoFBO = Loader::getFBO("AO");
            aoFBO->textures[0]->bind();
            loadUniform("aoOutput", aoFBO->textures[0]->textureId);
        }
};
