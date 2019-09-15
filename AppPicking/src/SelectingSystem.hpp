#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "MouseRayComponent.hpp"
#include "SelectedComponent.hpp"

using namespace neo;

template <typename CompT>
class SelectingSystem : public System {

public:
    SelectingSystem(
        int maxMarches = 100, 
        int maxDist = 100.f,
        std::function<bool(SelectedComponent*)> removeDecider = [](SelectedComponent*) { return true; }, 
        std::function<void(CompT*)> resetOperation = [](CompT*) {},
        std::function<void(SelectedComponent*)> selectOperation = [](SelectedComponent*) {},
        std::function<void(std::vector<SelectedComponent*>&)> editorOperation = [](std::vector<SelectedComponent*>&) {}) :
        
        System("Selecting System"),
        mMaxMarches(maxMarches),
        mMaxDist(maxDist),
        mRemoveDecider(removeDecider),
        mResetOperation(resetOperation),
        mSelectOperation(selectOperation),
        mEditorOperation(editorOperation)
    {}


    virtual void update(const float dt) override {
        // Select a new object
        if (auto mouseRay = Engine::getSingleComponent<MouseRayComponent>()) {
            for (auto selectable : Engine::getComponents<CompT>()) {
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
        for (auto selectable : Engine::getComponents<CompT>()) {
            mResetOperation(selectable);
        }

        // Operate on selected objects
        for (auto selected : Engine::getComponents<SelectedComponent>()) {
            mSelectOperation(selected);
        }
    }

    virtual void imguiEditor() override {
        auto selected = Engine::getComponents<SelectedComponent>();
        if (selected.size()) {
            mEditorOperation(selected);
        }
    }

private:
    const int mMaxMarches;
    const float mMaxDist;
    const std::function<bool(SelectedComponent*)> mRemoveDecider;
    const std::function<void(CompT*)> mResetOperation;
    const std::function<void(SelectedComponent*)> mSelectOperation;
    const std::function<void(std::vector<SelectedComponent*>&)> mEditorOperation;
};
