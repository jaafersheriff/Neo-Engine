#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLHelper/GLHelper.hpp"

using namespace neo;

class BlurShader : public PostProcessShader {

    public:

        int blurAmount = 6;

        BlurShader(const std::string &frag) :
            PostProcessShader("Blur Shader", frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            loadUniform("blurAmount", blurAmount);
        }
};