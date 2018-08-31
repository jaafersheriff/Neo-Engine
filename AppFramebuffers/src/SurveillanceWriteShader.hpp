#pragma once

#include <NeoEngine.hpp>
#include "Shader/Shader.hpp"

#include "SurveillanceCamera.hpp"

using namespace neo;

class SurveillanceWriteShader : public Shader {
    public: 
        SurveillanceWriteShader() :
            Shader("Surveillance Write")
        {}

        virtual void render(const CameraComponent &camera) override {
            auto cameras = NeoEngine::getComponents<SurveillanceCamera>();
            for (auto camera : cameras) {
                camera->fbo->bind();
                CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
                CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
                MasterRenderer::renderScene(*camera);
            }
        }
};