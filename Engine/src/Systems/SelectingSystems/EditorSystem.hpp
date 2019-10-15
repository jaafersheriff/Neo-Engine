#pragma once

#include "Systems/System.hpp"
#include "SelectingSystem.hpp"
#include "Component/SelectingComponent/SelectedComponent.hpp"

namespace neo {

    class EditorSystem : public SelectingSystem {

    public:
        EditorSystem() :
            SelectingSystem(20, 100,
                [](SelectedComponent*) { return true; }, // Always deselect on a new selection
                [](SelectableComponent* selectable) { },
                [](SelectedComponent* selected, glm::vec3 mousePos) {selected->getGameObject().getComponentByType<SpatialComponent>()->setPosition(mousePos); }
            )
        {}
    };

}
