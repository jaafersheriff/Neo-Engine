#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

using namespace neo;

class BlurShader : public PostProcessShader {

    public:

        int blurAmount = 6;

        BlurShader(const std::string &frag) :
            PostProcessShader("Blur Shader", frag) 
        {}

        virtual void render() override {
            loadUniform("blurAmount", blurAmount);
        }

        virtual void imguiEditor() override {
            ImGui::SliderInt("Blur", &blurAmount, 0, 10);
        }
};