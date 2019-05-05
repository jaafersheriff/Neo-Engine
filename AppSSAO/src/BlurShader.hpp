#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

class BlurShader : public neo::PostProcessShader {

    public:

        int blurAmount = 6;

        BlurShader(const std::string &frag) :
            neo::PostProcessShader("Blur Shader", frag) 
        {}

        virtual void render(const neo::CameraComponent &camera) override {
            loadUniform("blurAmount", blurAmount);
        }
};