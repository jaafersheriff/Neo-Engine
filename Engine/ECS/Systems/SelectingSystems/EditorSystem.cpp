#include "EditorSystem.hpp"
#include "Engine/Engine.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/RenderableComponent/OutlineRenderable.hpp"
#include "ECS/Component/SelectingComponent/MouseRayComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    EditorSystem::EditorSystem() 
        : SelectingSystem(
            "Editor System",
            [](ECS& ecs, ECS::Entity entity, SelectedComponent* reset) { 
                NEO_UNUSED(reset);
                ecs.removeComponent<renderable::OutlineRenderable>(entity);
            },
            [](ECS& ecs, ECS::Entity entity, SelectableComponent* selected) {
                NEO_UNUSED(selected);
                if (!ecs.has<renderable::OutlineRenderable>(entity)) {
                    ecs.addComponent<renderable::OutlineRenderable>(entity, glm::vec4(1.f, 0.95f, 0.72f, 1.f), 3.f);
                }
            },
            [](ECS& ecs, ECS::Entity entity, SelectedComponent* selected) {
                NEO_UNUSED(ecs, entity, selected);
            }
        )
    { 
    }

    // TODO : add hovered capability
    void EditorSystem::update(ECS& ecs) {
        if (auto tuple = ecs.getComponentTuple<SelectableComponent, SpatialComponent>()) {
            if (auto mouseRay = ecs.getComponent<MouseRayComponent>()) {
                if (auto mouse = ecs.getComponent<MouseComponent>()) {
                    auto&& [selectable, spatial] = tuple.get();
                    glm::vec3 pos;
                    if (auto bb = ecs.getComponent<BoundingBoxComponent>(tuple.mEntity)) {
                        glm::vec3 worldSpaceCenter = spatial.getModelMatrix() * glm::vec4(bb->getCenter(), 1.f);
                        glm::vec3 offsetTranslation = spatial.getPosition() - worldSpaceCenter;
                        float distance = glm::distance(worldSpaceCenter, mouseRay->mPosition);
                        distance += mouse->mFrameMouse.getScrollSpeed();
                        pos = mouseRay->mPosition + mouseRay->mDirection * distance + offsetTranslation;
                    }
                    else {
                        float distance = glm::distance(spatial.getPosition(), mouseRay->mPosition);
                        distance += mouse->mFrameMouse.getScrollSpeed();
                        pos = mouseRay->mPosition + mouseRay->mDirection * distance;
                    }
                    spatial.setPosition(pos);
                }
            }
        }
    }
}
