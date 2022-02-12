#include "EditorSystem.hpp"
#include "Engine/Engine.hpp"

namespace neo {

    EditorSystem::EditorSystem() 
        : SelectingSystem(
            "Editor System",
            [](ECS& ecs, SelectedComponent* reset) { 
                ecs.removeComponent(*reset->getGameObject().getComponentByType<renderable::OutlineRenderable>());
            },
            [](ECS& ecs, SelectableComponent* selected) {
                if (!selected->getGameObject().getComponentByType<renderable::OutlineRenderable>()) {
                    ecs.addComponent<renderable::OutlineRenderable>(&selected->getGameObject(), glm::vec4(1.f, 0.95f, 0.72f, 0.75f), 0.08f);
                }
            },
            [](ECS&, SelectedComponent* ) {}
        )
    { 
    }

    void EditorSystem::update(ECS& ecs) {
        if (auto selected = ecs.getSingleComponent<SelectedComponent>()) {
            if (auto spatial = selected->getGameObject().getComponentByType<SpatialComponent>()) {
                if (auto mouseRay = ecs.getSingleComponent<MouseRayComponent>()) {
                    if (auto mouse = ecs.getSingleComponent<MouseComponent>()) {
                        float offset = glm::distance(mouseRay->mPosition, spatial->getPosition());
                        offset += mouse->mFrameMouse.getScrollSpeed();
                        spatial->setPosition(mouseRay->mPosition + mouseRay->mDirection * offset);
                    }
                }
            }
        }
    }
}
