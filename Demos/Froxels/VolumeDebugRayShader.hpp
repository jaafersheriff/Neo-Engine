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

using namespace neo;

namespace Froxels {
    class VolumeDebugRayShader : public Shader {

    public:

        VolumeDebugRayShader(const std::string& vert, const std::string& frag) :
            Shader("VolumeDebugRay Shader", vert, frag) {
        }

        virtual void render(const ECS& ecs) override {
            bind();

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
                loadUniform("mockV", spatial.getView());
            }

            if (auto volumeOpt = ecs.getSingleView<VolumeComponent, SpatialComponent>()) {
                auto&& [_, volume, spatial] = *volumeOpt;

                loadUniform("M", spatial.getModelMatrix());
                loadTexture("volume", *volume.mTexture);

                /* DRAW */
                Library::getMesh("cube").mMesh->draw();
            }

            unbind();
        }
    };
}
