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

class DofInterpolateShader : public Shader {

    public:

        std::shared_ptr<int> frameScale;
        glm::vec3 interpolateBlur = glm::vec3(0.2f, 0.5f, 0.3f);
        glm::vec3 dofEqFar = glm::vec3(1.f);

        DofInterpolateShader(const std::string& vert, const std::string &frag, std::shared_ptr<int> scale) :
            Shader("DofInterpolate Shader", vert, frag),
            frameScale(scale)
        {
            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            auto DofInterpolateFBO = Library::getFBO("dofinterpolate");
            DofInterpolateFBO->attachColorTexture(frameSize, { GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT });
            DofInterpolateFBO->attachDepthTexture(frameSize, GL_NEAREST, GL_REPEAT); // depth
            DofInterpolateFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&, frameScale = frameScale](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("dofinterpolate")->resize(frameSize / glm::uvec2(*frameScale));
            });
 
        }

        virtual void render() override {
            Library::getFBO("dofinterpolate")->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
            CHECK_GL(glDisable(GL_DEPTH_TEST));

            glm::uvec2 frameSize = Window::getFrameSize() / *frameScale;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            loadUniform("invRenderTargetSize", glm::vec2(1.f) / glm::vec2(frameSize));
            loadUniform("dofLerpScale", glm::vec4( -1 / interpolateBlur.x, -1 / interpolateBlur.y, -1 / interpolateBlur.z, 1 / interpolateBlur.z ));
            loadUniform("dofLerpBias", glm::vec4(1.f, (1.f - interpolateBlur.y) / interpolateBlur.x, 1.f / interpolateBlur.y, (interpolateBlur.y - 1.f) / interpolateBlur.y));
            loadUniform("dofEqFar", dofEqFar);

            loadTexture("dofDown", *Library::getFBO("dofdown")->mTextures[0]);
            loadTexture("dofBlur", *Library::getFBO("dofnearblur")->mTextures[0]);

            loadTexture("inputFBO", *Library::getFBO("default")->mTextures[0]);
            loadTexture("inputDepth", *Library::getFBO("default")->mTextures[1]);

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            mesh->draw();
        }

        virtual void imguiEditor() override {
            if (ImGui::SliderFloat3("Interpolate Blur", &interpolateBlur[0], 0.f, 1.f)) {
                interpolateBlur = glm::normalize(interpolateBlur);
            }
            ImGui::SliderFloat3("Eq Far", &dofEqFar[0], 0.f, 1.f);
        }
};
