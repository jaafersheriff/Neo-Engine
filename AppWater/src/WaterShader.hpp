#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

#include "WaterMeshComponent.hpp"

using namespace neo;

class WaterShader : public Shader {

    public:

        WaterShader(const std::string &vert, const std::string &frag) :
            Shader("Water Shader", vert, frag) {
        }

        virtual void render(const CameraComponent &camera) override {
            bind();
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            if (auto renderable = Engine::getComponentTuple<WaterMeshComponent, SpatialComponent>()) {
                loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());

                renderable->get<WaterMeshComponent>()->getMesh().draw();
            }

            unbind();
        }
};