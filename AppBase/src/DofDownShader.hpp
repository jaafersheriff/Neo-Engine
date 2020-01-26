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

        glm::uvec2 frameSize = glm::uvec2(1920, 1080);
        glm::vec2 dofWorld = glm::vec2(0.3f, 0.5f);

        DofDownShader(const std::string& vert, const std::string &frag) :
            Shader("DofDown Shader", vert, frag)
        {
            auto DofDownFBO = Library::getFBO("dofdown");
            DofDownFBO->attachColorTexture(frameSize, { GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT });
            DofDownFBO->attachDepthTexture(frameSize, GL_NEAREST, GL_REPEAT); // depth
            DofDownFBO->initDrawBuffers();
        }

        virtual void render() override {
            Library::getFBO("dofdown")->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
            CHECK_GL(glDisable(GL_DEPTH_TEST));

            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            loadUniform("invRenderTargetSize", glm::vec2(1.f) / glm::vec2(frameSize));
            loadUniform("dofEqWorld", dofWorld);
            loadUniform("dofRowDelta", glm::vec2(0, 0.25f / frameSize.y));

            loadTexture("inputFBO", *Library::getFBO("default")->mTextures[0]);
            loadTexture("inputDepth", *Library::getFBO("default")->mTextures[1]);

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            mesh->draw();
        }

        virtual void imguiEditor() override {
            bool resizeFrame = false;
            glm::uvec2 minMax(128, 1920);
            resizeFrame |= ImGui::SliderScalar("Framesize.x", ImGuiDataType_U32, &frameSize.x, &minMax.x, &minMax.y);
            resizeFrame |= ImGui::SliderScalar("Framesize.y", ImGuiDataType_U32, &frameSize.y, &minMax.x, &minMax.y);
            if (resizeFrame) {
                Library::getFBO("DofDown")->resize(frameSize);
            }
            ImGui::SliderFloat2("DOF World", &dofWorld[0], 0.f, 100.f);
        }
};
