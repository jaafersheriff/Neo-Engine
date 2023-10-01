#include "FrustumCullingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/ObjectInMainView.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"

namespace neo {

    void FrustumCullingSystem::update(ECS& ecs) {
        NEO_ASSERT(ecs.isSystemEnabled<FrustumSystem>(), "This system can only be used with the FrustumSystem!");
        auto camera = ecs.getSingleView<MainCameraComponent, FrustumComponent>();

        for (auto&& [entity, spatial, bb] : ecs.getView<SpatialComponent, BoundingBoxComponent>().each()) {
            if (camera) {
                if (std::get<2>(*camera).isInFrustum(spatial, bb)) {
                    if (!ecs.cGetComponent<ObjectInMainViewComponent>(entity)) {
                        ecs.addComponent<ObjectInMainViewComponent>(entity);
                    }
                }
                else {
                    ecs.removeComponent<ObjectInMainViewComponent>(entity);
                }
            }
            else if (!ecs.cGetComponent<ObjectInMainViewComponent>(entity)) {
                ecs.addComponent<ObjectInMainViewComponent>(entity);
            }
        }
    }
}