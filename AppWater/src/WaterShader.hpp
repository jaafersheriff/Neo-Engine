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

        glm::vec3 tessFactor = glm::vec3(10.f, 7.f, 3.f);
        glm::vec2 tessDistance = glm::vec2(2.f, 5.f);

        WaterShader(const std::string &vert, const std::string &frag, const std::string& control, const std::string& eval) :
            Shader("Water Shader") {
            _attachType(vert, ShaderType::VERTEX);
            _attachType(frag, ShaderType::FRAGMENT);
            _attachType(control, ShaderType::TESSELLATION_CONTROL);
            _attachType(eval, ShaderType::TESSELLATION_EVAL);
            init();
        }

        virtual void render(const CameraComponent &camera) override {
            bind();
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());

            loadUniform("tessFactor", tessFactor);
            loadUniform("tessDistance", tessDistance);

            CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            CHECK_GL(glPatchParameteri(GL_PATCH_VERTICES, 3));
            if (auto renderable = Engine::getComponentTuple<WaterMeshComponent, SpatialComponent>()) {
                loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());

                renderable->get<WaterMeshComponent>()->getMesh().draw();
            }

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat3("TessFactor", &tessFactor[0], 0.1f, 10.f);
            ImGui::SliderFloat2("TessDistance", &tessDistance[0], 0.1f, 30.f);
        }
};