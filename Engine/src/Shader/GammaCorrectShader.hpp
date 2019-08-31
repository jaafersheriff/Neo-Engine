#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Loader/Library.hpp"
#include "Engine.hpp"

using namespace neo;

class GammaCorrectShader : public PostProcessShader {

    public:

        float gamma = 2.2f;

        GammaCorrectShader() :
            PostProcessShader("GammaCorrectShader", "gammacorrect.glsl") {
        }

        virtual void render(const CameraComponent &camera) override {
            loadUniform("gamma", gamma);
    }
};
