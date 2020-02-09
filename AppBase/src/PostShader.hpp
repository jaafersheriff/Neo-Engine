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

        glm::vec2 maxCoc = glm::vec2(5.f, 10.f);
        float radiusScale = 0.4f;

        PostShader(const std::string& frag) :
            PostProcessShader("Post Shader", frag)
        {}

        virtual void render() override {
            const auto& defaultFBO = *Library::getFBO("default")->mTextures[0];
            loadTexture("inputFBO", defaultFBO);
            loadUniform("scenePixelSize", 1.f / glm::vec2(defaultFBO.mWidth, defaultFBO.mHeight));

            const auto& dofBlur = *Library::getFBO("dofblur")->mTextures[0];
            loadTexture("dofblur", dofBlur);
            loadUniform("dofPixelSize", 1.f / glm::vec2(dofBlur.mWidth, dofBlur.mHeight));

            loadUniform("maxCoc", maxCoc);
            loadUniform("radiusScale", radiusScale);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat2("max coc", &maxCoc[0], 1.f, 30.f);
            ImGui::SliderFloat("radius scale", &radiusScale, 0.f, 1.f);
        }
};
