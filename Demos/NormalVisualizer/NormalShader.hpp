#pragma once

#include "Engine/Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

using namespace neo;

class NormalShader : public Shader {

    public: 
        float magnitude = 0.08f;
    
        NormalShader(const std::string &vert, const std::string &frag, const std::string &geom) :
            Shader("Normal Shader")
        {
            _attachStage(ShaderStage::VERTEX, vert);
            _attachStage(ShaderStage::FRAGMENT, frag);
            _attachStage(ShaderStage::GEOMETRY, geom);
            init();
        }

        virtual void render(const ECS& ecs) override {
            bind();

            loadUniform("magnitude", magnitude);

            /* Load PV */
            auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No MainCamera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            for (auto& renderable : ecs.getComponentTuples<MeshComponent, SpatialComponent>()) {
                loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());
                loadUniform("N", renderable->get<SpatialComponent>()->getNormalMatrix());

                /* DRAW */
                renderable->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Magnitude", &magnitude, 0.f, 1.f);
        }

};