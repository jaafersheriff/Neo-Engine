#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Library.hpp"
#include "GLObjects/Framebuffer.hpp"

using namespace neo;

class PostShader : public PostProcessShader {

    public:

        int showFBO = 2;
        float dofOpacity = 1.f;
        bool showDebug = false;

        PostShader(const std::string& frag) :
            PostProcessShader("Post Shader", frag)
        {}

        virtual void render() override {
            loadUniform("showDebug", showDebug);
            loadUniform("dofOpacity", dofOpacity);
            loadTexture("inputFBO", *Library::getFBO("default")->mTextures[0]);
            switch (showFBO) {
            case 0:
                loadTexture("inColor", *Library::getFBO("dofdown")->mTextures[0]);
                break;
            case 1:
                loadTexture("inColor", *Library::getFBO("dofnearblur")->mTextures[0]);
                break;
            case 2:
                loadTexture("inColor", *Library::getFBO("dofinterpolate")->mTextures[0]);
            default:
                break;
            }
        }

        virtual void imguiEditor() override {
            ImGui::Checkbox("Debug", &showDebug);
            if (showDebug) {
                if (ImGui::RadioButton("Show Down", showFBO == 0)) {
                    showFBO = 0;
                }
                if (ImGui::RadioButton("Show Blur", showFBO == 1)) {
                    showFBO = 1;
                }
                if (ImGui::RadioButton("Show Interpolate", showFBO == 2)) {
                    showFBO = 2;
                }
            }
            ImGui::SliderFloat("Opacity", &dofOpacity, 0.f, 5.f);
        }
};
