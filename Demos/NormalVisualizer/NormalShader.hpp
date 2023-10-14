#pragma once

#include "Engine/Engine.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include <imgui.h>
#include <tracy/TracyOpenGL.hpp>

using namespace neo;

namespace NormalVisualizer {
    class NormalShader : public Shader {

    public:
        float magnitude = 0.08f;

        NormalShader(const std::string& vert, const std::string& frag, const std::string& geom) :
            Shader("Normal Shader")
        {
            _attachStage(ShaderStage::VERTEX, vert);
            _attachStage(ShaderStage::FRAGMENT, frag);
            _attachStage(ShaderStage::GEOMETRY, geom);
            init();
        }

        virtual void render(const ECS& ecs) override {
            ZoneScoped;
            TracyGpuZone("NormalShader");
            bind();

            loadUniform("magnitude", magnitude);

            /* Load PV */
            const auto& camera = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            NEO_ASSERT(camera, "No main camera :(");
            auto&& [cameraEntity, _, cameraSpatial] = *camera;
            loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
            loadUniform("V", cameraSpatial.getView());

            for (const auto&& [__, mesh, spatial] : ecs.getView<MeshComponent, SpatialComponent>().each()) {
                loadUniform("M", spatial.getModelMatrix());
                loadUniform("N", spatial.getNormalMatrix());

                /* DRAW */
                mesh.mMesh->draw();
            }

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Magnitude", &magnitude, 0.f, 1.f);
        }

    };
}