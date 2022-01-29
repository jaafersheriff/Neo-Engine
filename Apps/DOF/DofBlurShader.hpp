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

class DofBlurShader : public Shader {

    public:

        std::shared_ptr<int> frameScale;
        int blurSize = 1;

        DofBlurShader(const std::string& vert, const std::string &frag, std::shared_ptr<int> scale) :
            Shader("DofBlur Shader", vert, frag),
            frameScale(scale)
        {
            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            auto DofBlurFBO = Library::createFBO("dofblur");
            DofBlurFBO->attachColorTexture(frameSize, { GL_RGBA, GL_RGBA, GL_NEAREST, GL_CLAMP_TO_EDGE });
            DofBlurFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&, frameScale = frameScale](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                NEO_UNUSED(m);
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("dofblur")->resize(frameSize / glm::uvec2(*frameScale));
            });
 
        }

        virtual void render() override {
            Library::getFBO("dofblur")->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            auto dofdownFBO = Library::getFBO("dofdown")->mTextures[0];
            loadTexture("dofDown", *dofdownFBO);
            loadUniform("frameSize", glm::ivec2(dofdownFBO->mWidth, dofdownFBO->mHeight));
            loadUniform("blurSize", blurSize);

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            mesh->draw();
        }

        virtual void imguiEditor() override {
            ImGui::SliderInt("Blur size", &blurSize, 0, 8);
        }
};
