#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Library.hpp"
#include "GLObjects/Framebuffer.hpp"
#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

using namespace neo;

class DofBlurShader : public Shader {

    public:

        std::shared_ptr<int> frameScale;

        DofBlurShader(const std::string& vert, const std::string &frag, std::shared_ptr<int> scale) :
            Shader("DofBlur Shader", vert, frag),
            frameScale(scale)
        {
            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            auto DofBlurFBO = Library::getFBO("dofblur");
            DofBlurFBO->attachColorTexture(frameSize, { GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT });
            DofBlurFBO->attachDepthTexture(frameSize, GL_NEAREST, GL_REPEAT); // depth
            DofBlurFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&, frameScale = frameScale](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("dofblur")->resize(frameSize / glm::uvec2(*frameScale));
            });
 
        }

        virtual void render() override {
            Library::getFBO("dofblur")->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
            CHECK_GL(glDisable(GL_DEPTH_TEST));

            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            loadUniform("invRenderTargetSize", glm::vec2(1.f) / glm::vec2(frameSize));

            loadTexture("dofDown", *Library::getFBO("dofdown")->mTextures[0]);

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            mesh->draw();
        }

        virtual void imguiEditor() override {
        }
};
