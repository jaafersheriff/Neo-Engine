#include "EditorSystem.hpp"
#include "Engine/Engine.hpp"

namespace neo {

    EditorSystem::EditorSystem() 
        : SelectingSystem(
            "Editor System",
            25,
            100.f,
            [](SelectedComponent* selected) { 
                NEO_UNUSED(selected);
                return true; 
            },
            [](SelectableComponent* selectable) { 
                Engine::removeComponent(*selectable->getGameObject().getComponentByType<renderable::OutlineRenderable>());
            },
            [](SelectedComponent* selected, const MouseRayComponent* mouseRay, float delta) {
                NEO_UNUSED(delta);
                if (!selected->getGameObject().getComponentByType<renderable::OutlineRenderable>()) {
                    Engine::addComponent<renderable::OutlineRenderable>(&selected->getGameObject(), glm::vec4(1.f, 0.95f, 0.72f, 0.75f), 0.08f);
                }
                if (auto spatial = selected->getGameObject().getComponentByType<SpatialComponent>()) {  
                    spatial->setPosition(mouseRay->mPosition + mouseRay->mDirection * glm::distance(mouseRay->mPosition, spatial->getPosition()));
                }
            }) 
    { }
}
