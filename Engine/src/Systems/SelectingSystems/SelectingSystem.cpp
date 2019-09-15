#include <Engine.hpp>
#include "SelectingSystem.hpp"

namespace neo {

    void SelectingSystem::update(const float dt) {
        // Select a new object
        if (auto mouseRay = Engine::getSingleComponent<MouseRayComponent>()) {
            for (auto selectable : Engine::getComponents<SelectableComponent>()) {
                if (auto bb = selectable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                    float maxDistance = mMaxDist;
                    if (auto camera = Engine::getSingleComponent<CameraComponent>()) {
                        maxDistance = glm::max(maxDistance, camera->getNearFar().y);
                    }

                    // Ray march
                    for (float i = 0.f; i < maxDistance; i += maxDistance / static_cast<float>(mMaxMarches)) {
                        if (bb->intersect(mouseRay->position + mouseRay->ray * i)) {

                            // Decide to remove unselected objecys
                            for (auto selected : Engine::getComponents<SelectedComponent>()) {
                                if (mRemoveDecider(selected)) {
                                    Engine::removeComponent<SelectedComponent>(*selected);
                                }
                            }
                            Engine::addComponent<SelectedComponent>(&selectable->getGameObject());
                        }
                    }
                }
            }
        }

        // Operate on unselected objects
        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            mResetOperation(selectable);
        }

        // Operate on selected objects
        for (auto selected : Engine::getComponents<SelectedComponent>()) {
            mSelectOperation(selected);
        }
    }

    void SelectingSystem::imguiEditor() {
        auto selected = Engine::getComponents<SelectedComponent>();
        if (selected.size()) {
            mEditorOperation(selected);
        }
    }

}
