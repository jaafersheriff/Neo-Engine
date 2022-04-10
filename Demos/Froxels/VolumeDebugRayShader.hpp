#pragma once

#include "Renderer/GLObjects/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "VolumeComponent.hpp"
#include "VolumeWriteCameraComponent.hpp"

#include "Loader/Library.hpp"

#include <imgui/imgui.h>

using namespace neo;

namespace Froxels {
    class VolumeDebugRayShader : public Shader {

    public:

        VolumeDebugRayShader(const std::string& vert, const std::string& frag) :
            Shader("VolumeDebugRay Shader", vert, frag) {
        }

        virtual void render(const ECS& ecs) override {
            bind();

            loadUniform("StepSize", stepSize);
            loadUniform("Iterations", iterations);

            // Main camera PV, to draw the box
            if (auto cameraOpt = ecs.getSingleView<MainCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, spatial] = *cameraOpt;
                loadUniform("P", camera.getProj());
                loadUniform("V", spatial.getView());
            }

            // Mock camera PV, to color the box
            if (auto cameraOpt = ecs.getSingleView<VolumeWriteCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, spatial] = *cameraOpt;
                loadUniform("mockP", camera.getProj());
                loadUniform("near", camera.getNearFar().x);
                loadUniform("far", camera.getNearFar().y);
                loadUniform("mockV", spatial.getView());
                loadUniform("camPos", spatial.getPosition());
            }

            if (auto volumeOpt = ecs.getSingleView<VolumeComponent, SpatialComponent>()) {
                auto&& [_, volume, spatial] = *volumeOpt;

                loadUniform("M", spatial.getModelMatrix());
                loadTexture("volume", *volume.mTexture);

                /* DRAW */

                glFrontFace(GL_CCW);
                Library::getMesh("cube").mMesh->draw();

                glFrontFace(GL_CW);
                Library::getMesh("cube").mMesh->draw();
            }

            unbind();
        }

        void imguiEditor() {
            ImGui::SliderFloat("Step size", &stepSize, 0.001f, 10.f);
            ImGui::SliderInt("Iterations", &iterations, 0, 16);
        }

        float stepSize = 0.01f;
        int iterations = 1;
    };
}
