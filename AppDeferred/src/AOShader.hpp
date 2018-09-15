#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"
#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class AOShader : public Shader {

    public:

        AOShader(const std::string &frag) :
            Shader("AO Shader", MasterRenderer::POST_PROCESS_VERT_FILE, frag) 
        {
            // Create render target
            auto aoFBO = Loader::getFBO("AO");
            aoFBO->generate();
            aoFBO->attachColorTexture(Window::getFrameSize(), 1, GL_R16, GL_RED, GL_NEAREST, GL_REPEAT); // ao

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                auto aoFBO = Loader::getFBO("AO");
                aoFBO->textures[0]->width  = frameSize.x;
                aoFBO->textures[0]->height = frameSize.y;
                aoFBO->textures[0]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, frameSize.x, frameSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr));
            });
        }

        virtual void render(const CameraComponent &camera) override {
            // TODO - set up post process 
        }
};
