#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class NormalShader : public Shader {

    public: 
        float magnitude = 0.4f;
    
        NormalShader(const std::string &vert, const std::string &frag, const std::string &geom) :
            Shader("Normal Shader")
        {
            _attachStage(ShaderStage::VERTEX, vert);
            _attachStage(ShaderStage::FRAGMENT, frag);
            _attachStage(ShaderStage::GEOMETRY, geom);
            init();
        }

        virtual void render(const CameraComponent &camera) override {
            bind();

            loadUniform("magnitude", magnitude);

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto& renderable : Engine::getComponentTuples<MeshComponent, SpatialComponent>()) {
                loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());
                loadUniform("N", renderable->get<SpatialComponent>()->getNormalMatrix());

                /* DRAW */
                renderable->get<MeshComponent>()->getMesh().draw();
            }

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Magnitude", &magnitude, 0.f, 1.f);
        }

};