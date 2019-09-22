#include <Engine.hpp>
#include "SelectingSystem.hpp"

namespace neo {

    void SelectingSystem::init() {
        // Operate on unselected objects
        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            mResetOperation(selectable);
        }
    }

    void SelectingSystem::update(const float dt) {
        if (auto mouseRay = Engine::getSingleComponent<MouseRayComponent>()) {
            // Decide to remove unselected objects
            for (auto selected : Engine::getComponents<SelectedComponent>()) {
                if (mRemoveDecider(selected)) {
                    Engine::removeComponent<SelectedComponent>(*selected);
                }
            }

            float maxDistance = mMaxDist;
            auto camera = Engine::getSingleComponent<CameraComponent>();
            if (camera) {
                maxDistance = glm::max(maxDistance, camera->getNearFar().y);
            }

            // Select a new object
            for (auto selectable : Engine::getComponents<SelectableComponent>()) {
                if (auto bb = selectable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                    // Frustum culling
                    if (const auto& spatial = selectable->getGameObject().getComponentByType<SpatialComponent>()) {
                        if (camera) {
                            if (const auto& frustumPlanes = camera->getGameObject().getComponentByType<FrustumComponent>()) {
                                float radius = glm::max(glm::max(spatial->getScale().x, spatial->getScale().y), spatial->getScale().z) * bb->getRadius();
                                if (!frustumPlanes->isInFrustum(spatial->getPosition(), radius)) {
                                    continue;
                                }
                            }
                        }
                    }


                    // Ray march
                    for (float i = 0.f; i < maxDistance; i += maxDistance / static_cast<float>(mMaxMarches)) {
                        if (bb->intersect(mouseRay->position + mouseRay->direction * i)) {
                            // Add and operate on new selected
                            mSelectOperation(&Engine::addComponent<SelectedComponent>(&selectable->getGameObject()));
                        }
                    }
                }
            }

            // Operate on unselected objects
            for (auto selectable : Engine::getComponents<SelectableComponent>()) {
                if (!selectable->getGameObject().getComponentByType<SelectedComponent>()) {
                    mResetOperation(selectable);
                }
            }
        }
    }

    void SelectingSystem::imguiEditor() {
        auto selected = Engine::getComponents<SelectedComponent>();
        if (selected.size()) {
            mEditorOperation(selected);
        }
    }

}
