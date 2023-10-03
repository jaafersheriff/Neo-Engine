#include "FrustumCullingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"

#include <imgui/imgui.h>

namespace neo {

    void FrustumCullingSystem::update(ECS& ecs) {
        NEO_ASSERT(ecs.isSystemEnabled<FrustumSystem>(), "This system can only be used with the FrustumSystem!");
        mCulledCount = 0;

        auto cameras = ecs.getView<FrustumComponent>();

        for (auto&& [entity, spatial, bb] : ecs.getView<SpatialComponent, BoundingBoxComponent>().each()) {
            CameraCulledComponent culled;
            for(auto&& [cameraEntity, frustum] : cameras.each()) {
                if ((ecs.has<PerspectiveCameraComponent>(cameraEntity) || ecs.has<OrthoCameraComponent>(cameraEntity)) && frustum.isInFrustum(spatial, bb)) {
                    culled.mCameraViews.insert(cameraEntity);
                }
                else {
                    mCulledCount++;
                }
            }
            if (auto* existingComp = ecs.getComponent<CameraCulledComponent>(entity)) {
                existingComp->mCameraViews = culled.mCameraViews;
            }
            else {
                ecs.addComponent<CameraCulledComponent>(entity, std::move(culled));
            }
        }
    }

    void FrustumCullingSystem::imguiEditor(ECS&) {
        ImGui::Text("Culled draws: %d", mCulledCount);
    }
}