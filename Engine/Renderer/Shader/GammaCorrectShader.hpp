#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Loader/Library.hpp"
#include "Engine/Engine.hpp"

using namespace neo;

class GammaCorrectShader : public PostProcessShader {

    public:

        float gamma = 2.2f;

        GammaCorrectShader() :
            PostProcessShader("GammaCorrectShader", std::string("gammacorrect.glsl")) {
        }

        virtual void render() override {
            loadUniform("gamma", gamma);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Gamma", &gamma, 0.f, 5.f);
        }
};
