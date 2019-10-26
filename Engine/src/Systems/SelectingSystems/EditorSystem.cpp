#include "EditorSystem.hpp"
#include <Engine.hpp>

namespace neo {

    EditorSystem::EditorSystem() 
        : SelectingSystem(
            25,
            100.f,
            [](SelectedComponent* selected) { 
                return true; 
            },
            [](SelectableComponent* selectable) { 
                Engine::removeComponent(*selectable->getGameObject().getComponentByType<renderable::OutlineRenderable>());
            },
            [](SelectedComponent* selected, glm::vec3 mousePos) {
                if (!selected->getGameObject().getComponentByType<renderable::OutlineRenderable>()) {
                    Engine::addComponent<renderable::OutlineRenderable>(&selected->getGameObject(), glm::vec4(1.f, 0.95f, 0.72f, 0.75f), 0.08f);
                }
            }) 
    { }
}
