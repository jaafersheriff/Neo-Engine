#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

using namespace neo;

class BlurShader : public Shader {

    public:

        int blurAmount = 1;

        BlurShader(const std::string &frag) :
            Shader("Blur Shader", MasterRenderer::POST_PROCESS_VERT_FILE, frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            loadUniform("blurAmount", blurAmount);
        }
};