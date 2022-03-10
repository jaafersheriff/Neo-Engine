#include "FrustumToLineSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"

namespace neo {

    void FrustumToLineSystem::update(ECS& ecs) {
        for (auto& camera : ecs.getComponentTuples<CameraComponent, LineMeshComponent, FrustumComponent>()) {
            auto&& [_, line, frustum] = camera.get();

            line.clearNodes();

            // We have to do this ridiculousness because line strip (:
            line.addNode(frustum.NearLeftBottom);
            line.addNode(frustum.NearLeftTop);
            line.addNode(frustum.NearRightTop);
            line.addNode(frustum.NearRightBottom);
            line.addNode(frustum.NearLeftBottom);
            line.addNode(frustum.FarLeftBottom);
            line.addNode(frustum.FarLeftTop);
            line.addNode(frustum.NearLeftTop);
            line.addNode(frustum.FarLeftTop);
            line.addNode(frustum.FarRightTop);
            line.addNode(frustum.NearRightTop);
            line.addNode(frustum.FarRightTop);
            line.addNode(frustum.FarRightBottom);
            line.addNode(frustum.NearRightBottom);
            line.addNode(frustum.FarRightBottom);
            line.addNode(frustum.FarLeftBottom);
        }
    }
}
