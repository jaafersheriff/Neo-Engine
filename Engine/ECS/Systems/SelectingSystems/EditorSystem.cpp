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
            [](ECS& ecs, SelectedComponent* reset) { 
                ecs.removeComponent(*reset->getGameObject().getComponentByType<renderable::OutlineRenderable>());
            },
            [](ECS& ecs, SelectableComponent* selected) {
                if (!selected->getGameObject().getComponentByType<renderable::OutlineRenderable>()) {
                    ecs.addComponent<renderable::OutlineRenderable>(&selected->getGameObject(), glm::vec4(1.f, 0.95f, 0.72f, 1.f), 3.f);
                }
            },
            [](ECS&, SelectedComponent* ) {}
        )
    { 
    }

    // TODO : add hovered capability
    void EditorSystem::update(ECS& ecs) {
        if (auto selected = ecs.getSingleComponent<SelectedComponent>()) {
            if (auto spatial = selected->getGameObject().getComponentByType<SpatialComponent>()) {
                if (auto mouseRay = ecs.getSingleComponent<MouseRayComponent>()) {
                    if (auto mouse = ecs.getSingleComponent<MouseComponent>()) {
                        glm::vec3 pos;
                        if (auto bb = selected->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                            glm::vec3 worldSpaceCenter = spatial->getModelMatrix() * glm::vec4(bb->getCenter(), 1.f);
                            glm::vec3 offsetTranslation = spatial->getPosition() - worldSpaceCenter;
                            float distance = glm::distance(worldSpaceCenter, mouseRay->mPosition);
                            distance += mouse->mFrameMouse.getScrollSpeed();
                            pos = mouseRay->mPosition + mouseRay->mDirection * distance + offsetTranslation;
                        }
                        else {
                            float distance = glm::distance(spatial->getPosition(), mouseRay->mPosition);
                            distance += mouse->mFrameMouse.getScrollSpeed();
                            pos = mouseRay->mPosition + mouseRay->mDirection * distance;
                        }
                        spatial->setPosition(pos);
                    }
                }
            }
        }
    }
}
