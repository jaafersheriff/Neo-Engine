#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/ECS.hpp"
#include "ParticleMeshComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

namespace Compute {
    class ParticleVisShader : public Shader {

    public:

        float mSpriteSize = 0.2f;
        glm::vec3 mSpriteColor = glm::vec3(0.67f, 1.f, 0.55f);

        ParticleVisShader(const std::string& vert, const std::string& frag, const std::string& geom) :
            Shader("ParticleVis Shader")
        {
            _attachStage(ShaderStage::VERTEX, vert);
            _attachStage(ShaderStage::FRAGMENT, frag);
            _attachStage(ShaderStage::GEOMETRY, geom);
            init();
        }

        virtual void render(const ECS& ecs) override {
            ZoneScoped;
            TracyGpuZone("ParticleVisShader");
            bind();

            if (auto cameraView = ecs.getSingleView<MainCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, camSpatial] = *cameraView;
                loadUniform("P", camera.getProj());
                loadUniform("V", camSpatial.getView());
            }
            loadUniform("spriteSize", mSpriteSize);
            loadUniform("spriteColor", mSpriteColor);

            if (auto meshView = ecs.getSingleView<ParticleMeshComponent, SpatialComponent>()) {
                auto&& [_, mesh, spatial] = *meshView;
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);

                loadUniform("M", spatial.getModelMatrix());

                /* DRAW */
                mesh.mMesh->draw();
            }
            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Sprite size", &mSpriteSize, 0.1f, 2.f);
            ImGui::ColorEdit3("Sprite color", &mSpriteColor[0]);
        }
    };
}
