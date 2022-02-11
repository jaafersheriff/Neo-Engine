#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Library.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

using namespace neo;

class DofDownShader : public Shader {

    public:

        std::shared_ptr<int> frameScale;

        DofDownShader(const std::string& vert, const std::string &frag, std::shared_ptr<int> scale) :
            Shader("DofDown Shader", vert, frag),
            frameScale(scale)
        {
            glm::uvec2 frameSize = WindowSurface::getFrameSize() / *frameScale;
            auto DofDownFBO = Library::createFBO("dofdown");
            DofDownFBO->attachColorTexture(frameSize, { GL_RGBA, GL_RGBA, GL_NEAREST, GL_CLAMP_TO_EDGE });
            DofDownFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&, frameScale = frameScale](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                NEO_UNUSED(m);
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("dofdown")->resize(frameSize / glm::uvec2(*frameScale));
            });
 
        }

        virtual void render() override {
            Library::getFBO("dofdown")->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            glm::uvec2 frameSize = WindowSurface::getFrameSize() / *frameScale;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            loadTexture("inputFBO", *Library::getFBO("default")->mTextures[0]);
            loadTexture("inputDepth", *Library::getFBO("default")->mTextures[1]);

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            mesh->draw();
        }

        virtual void imguiEditor() override {
        }
};
