#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "MouseRayComponent.hpp"
#include "SelectedComponent.hpp"

using namespace neo;

template <typename CompT>
class SelectingSystem : public System {

public:
    SelectingSystem(std::function<bool(SelectedComponent*)> removeDecider, int maxMarches = 100, float maxDist = 100.f) :
        System("Selecting System"),
        mRemoveDecider(removeDecider),
        mMaxMarches(maxMarches),
        mMaxDist(maxDist)
    {}


    virtual void update(const float dt) override {
        auto mouseRay = Engine::getSingleComponent<MouseRayComponent>();
        if (!mouseRay || !Mouse::isDown(GLFW_MOUSE_BUTTON_1)) {
            return;
        }

        for (auto selectable : Engine::getComponents<CompT>()) {
            if (auto bb = selectable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                float maxDistance = mMaxDist;
                if (auto camera = Engine::getSingleComponent<CameraComponent>()) {
                    maxDistance = glm::max(maxDistance, camera->getNearFar().y);
                }

                // ray march
                for (float i = 0.f; i < maxDistance; i += maxDistance / static_cast<float>(mMaxMarches)) {
                    if (bb->intersect(mouseRay->position + mouseRay->ray * i)) {
                        for (auto selected : Engine::getComponents<SelectedComponent>()) {
                            if (mRemoveDecider(selected)) {
                                Engine::removeComponent<SelectedComponent>(*selected);
                            }
                        }
                        Engine::addComponent<SelectedComponent>(&selectable->getGameObject());
                        return;
                    }
                }
            }
        }
    }

private:
    std::function<bool(SelectedComponent*)> mRemoveDecider;
    int mMaxMarches;
    float mMaxDist;
};
