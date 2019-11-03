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
            auto mainCamera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            if (mainCamera) {
                maxDistance = glm::max(maxDistance, mainCamera->get<CameraComponent>()->getNearFar().y);
            }

            // Select a new object
            for (auto& selectable : Engine::getComponentTuples<SelectableComponent, BoundingBoxComponent, SpatialComponent>()) {
                auto selectableSpatial = selectable->get<SpatialComponent>();
                auto selectableBox = selectable->get<BoundingBoxComponent>();

                // Frustum culling
                if (mainCamera) {
                    if (const auto& frustumPlanes = mainCamera->mGameObject.getComponentByType<FrustumComponent>()) {
                        float radius = glm::max(glm::max(selectableSpatial->getScale().x, selectableSpatial->getScale().y), selectableSpatial->getScale().z) * selectableBox->getRadius();
                        if (!frustumPlanes->isInFrustum(selectableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }


                // Ray march
                bool objectFound = false;
                for (float i = 0.f; i < maxDistance; i += maxDistance / static_cast<float>(mMaxMarches)) {
                    glm::vec3 raySample = mouseRay->position + mouseRay->direction * i;
                    if (selectableBox->intersect(raySample)) {
                        // Add and operate on new selected
                        mSelectOperation(&Engine::addComponent<SelectedComponent>(&selectable->mGameObject), raySample);
                        objectFound = true;
                        break;
                    }
                }
                if (objectFound) {
                    break;
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

    void SelectingSystem::imguiEditor() {
        auto selected = Engine::getComponents<SelectedComponent>();
        if (selected.size()) {
            mEditorOperation(selected);
        }
    }

}
