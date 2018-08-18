#pragma once

using namespace neo;

#include <NeoEngine.hpp>
#include "Shader/Shader.hpp"

#include "SurveillanceCamera.hpp"

class SurveillanceWriteShader : public Shader {
    public: 
        SurveillanceWriteShader(const std::string &res) :
            Shader("Surveillance Write")
        {}

        virtual void render(const RenderSystem &rSystem, const CameraComponent &camera) override {
            auto cameras = NeoEngine::getComponents<SurveillanceCamera>();
            for (auto camera : cameras) {
                camera->fbo->bind();
                CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
                CHECK_GL(glViewport(0, 0, camera->fboTex->width, camera->fboTex->height));
                rSystem.renderScene(*camera);
            }
        }
};