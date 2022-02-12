#include "FrustumToLineSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"

namespace neo {

    void FrustumToLineSystem::update(ECS& ecs) {
        for (auto& camera : ecs.getComponentTuples<CameraComponent, LineMeshComponent, FrustumComponent>()) {

            auto line = camera->get<LineMeshComponent>();
            auto bounds = camera->get<FrustumComponent>();

            line->clearNodes();

            // We have to do this ridiculousness because line strip (:
            line->addNode(bounds->NearLeftBottom);
            line->addNode(bounds->NearLeftTop);
            line->addNode(bounds->NearRightTop);
            line->addNode(bounds->NearRightBottom);
            line->addNode(bounds->NearLeftBottom);
            line->addNode(bounds->FarLeftBottom);
            line->addNode(bounds->FarLeftTop);
            line->addNode(bounds->NearLeftTop);
            line->addNode(bounds->FarLeftTop);
            line->addNode(bounds->FarRightTop);
            line->addNode(bounds->NearRightTop);
            line->addNode(bounds->FarRightTop);
            line->addNode(bounds->FarRightBottom);
            line->addNode(bounds->NearRightBottom);
            line->addNode(bounds->FarRightBottom);
            line->addNode(bounds->FarLeftBottom);
        }
    }
}
