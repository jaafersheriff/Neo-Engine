#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Library.hpp"
#include "GLObjects/Framebuffer.hpp"

#include "ext/imgui/imgui.h"

using namespace neo;

class DofDownShader : public Shader {

    public:

        std::shared_ptr<int> frameScale;
        glm::vec3 focalPoints = glm::vec3(0.f, 0.5f, 1.f);

        DofDownShader(const std::string& vert, const std::string &frag, std::shared_ptr<int> scale) :
            Shader("DofDown Shader", vert, frag),
            frameScale(scale)
        {
            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            auto DofDownFBO = Library::getFBO("dofdown");
            DofDownFBO->attachColorTexture(frameSize, { GL_RG8, GL_RG, GL_NEAREST, GL_CLAMP_TO_EDGE });
            DofDownFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&, frameScale = frameScale](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("dofdown")->resize(frameSize / glm::uvec2(*frameScale));
            });
 
        }

        virtual void render() override {
            /* TODO 
            This could be done purely as post process 
            Renderer would just need to resize ping/pong everytime?
            This shader would need to output to pong and a custom RT though 
            pong for DofNearBlur
            custom RT for DofInterpolate */
            Library::getFBO("dofdown")->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 0.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            loadUniform("focalPoints", focalPoints);

            loadTexture("inputFBO", *Library::getFBO("default")->mTextures[0]);
            loadTexture("inputDepth", *Library::getFBO("default")->mTextures[1]);

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            mesh->draw();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat3("Focal points", &focalPoints[0], 0.f, 1.f);
        }
};
