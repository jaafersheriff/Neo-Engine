#include "EditorSystem.hpp"
#include "Engine/Engine.hpp"

namespace neo {

    EditorSystem::EditorSystem() 
        : SelectingSystem(
            "Editor System",
            [](SelectedComponent* reset) { 
                Engine::removeComponent(*reset->getGameObject().getComponentByType<renderable::OutlineRenderable>());
            },
            [](SelectableComponent* selected) {
                if (!selected->getGameObject().getComponentByType<renderable::OutlineRenderable>()) {
                    Engine::addComponent<renderable::OutlineRenderable>(&selected->getGameObject(), glm::vec4(1.f, 0.95f, 0.72f, 0.75f), 0.08f);
                }
            },
            [](SelectedComponent* ) {}
        )
    { 
    }

    void EditorSystem::update(float dt) {
        NEO_UNUSED(dt);
        if (auto selected = Engine::getSingleComponent<SelectedComponent>()) {
            if (auto spatial = selected->getGameObject().getComponentByType<SpatialComponent>()) {
                if (auto mouseRay = Engine::getSingleComponent<MouseRayComponent>()) {
                    spatial->setPosition(mouseRay->mPosition + mouseRay->mDirection * glm::distance(mouseRay->mPosition, spatial->getPosition()));
                }
            }
        }
    }
}
