#include "ECS/pch.hpp"
#include "FrustumCullingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"

namespace neo {

    void FrustumCullingSystem::update(ECS& ecs) {
        TRACY_ZONEN("FrustumCullingSystem");
        NEO_ASSERT(ecs.isSystemEnabled<FrustumSystem>(), "This system can only be used with the FrustumSystem!");
        mCulledCount = 0;

        auto cameras = ecs.getView<FrustumComponent>();

        std::vector<ECS::Entity> cameraViews;
        cameraViews.reserve(cameras.size());
        for (auto&& [entity, spatial, bb] : ecs.getView<SpatialComponent, BoundingBoxComponent>().each()) {
            // cameraViews.clear();
            for (auto camera = cameraViews.begin(); camera < cameraViews.end(); camera++) {
                *camera = static_cast<ECS::Entity>(std::numeric_limits<uint32_t>::max());
            }

            for(auto&& [cameraEntity, frustum] : cameras.each()) {
                if ((ecs.has<PerspectiveCameraComponent>(cameraEntity) || ecs.has<OrthoCameraComponent>(cameraEntity)) && frustum.isInFrustum(spatial, bb)) {
                    cameraViews.push_back(cameraEntity);
                }
                else {
                    mCulledCount++;
                }
            }
            if (auto* existingComp = ecs.getComponent<CameraCulledComponent>(entity)) {
                existingComp->mCameraViews.swap(cameraViews);
            }
            else {
                ecs.addComponent<CameraCulledComponent>(entity, cameraViews);
            }
        }
    }

    void FrustumCullingSystem::imguiEditor(ECS&) {
        ImGui::Text("Culled draws: %d", mCulledCount);
    }
}