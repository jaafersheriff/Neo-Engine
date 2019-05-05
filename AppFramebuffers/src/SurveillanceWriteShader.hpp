#pragma once

#include <Engine.hpp>
#include "Shader/Shader.hpp"

#include "SurveillanceCamera.hpp"

class SurveillanceWriteShader : public neo::Shader {
    public: 
        SurveillanceWriteShader() :
            neo::Shader("Surveillance Write")
        {}

        virtual void render(const neo::CameraComponent &camera) override {
            auto cameras = neo::Engine::getComponents<SurveillanceCamera>();
            for (auto camera : cameras) {
                camera->fbo->bind();
                CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
                CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
                neo::Renderer::renderScene(*camera);
            }
        }
};