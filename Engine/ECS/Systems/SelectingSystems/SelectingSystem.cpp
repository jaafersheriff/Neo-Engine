#include "Engine/Engine.hpp"
#include "SelectingSystem.hpp"

namespace neo {

    void SelectingSystem::init() {
        // Operate on unselected objects
        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            mResetOperation(selectable);
        }
    }

    void SelectingSystem::update(const float dt) {
        NEO_UNUSED(dt);
        if (auto mouseRay = Engine::getSingleComponent<MouseRayComponent>()) {

            SelectableComponent* selectedSelectable = nullptr;
            float intersectDist = 0.f;
            float maxDistance = mMaxDist;
            auto mainCamera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            if (mainCamera) {
                maxDistance = glm::min(maxDistance, mainCamera->get<CameraComponent>()->getNearFar().y);
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
                for (float i = 0.f; i < maxDistance; i += maxDistance / static_cast<float>(mMaxMarches)) {
                    glm::vec3 raySample = mouseRay->mPosition + mouseRay->mDirection * i;
                    if (selectableBox->intersect(raySample)) {
                        selectedSelectable = selectable->get<SelectableComponent>();
                        intersectDist = i;
                        break;
                    }
                }
                if (selectedSelectable) {
                    break;
                }
            }


            SelectedComponent* selected = nullptr;
            if (selectedSelectable) {
                selected = selectedSelectable->getGameObject().getComponentByType<SelectedComponent>();
                if (!selected) {
                    selected = &Engine::addComponent<SelectedComponent>(&selectedSelectable->getGameObject());
                }
                mSelectOperation(selected, mouseRay, intersectDist);
            }

            // Decide to remove unselected objects
            for (auto eSelected : Engine::getComponents<SelectedComponent>()) {
                if ((&eSelected->getGameObject() != &selected->getGameObject()) && mRemoveDecider(eSelected)) {
                    Engine::removeComponent<SelectedComponent>(*eSelected);
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
