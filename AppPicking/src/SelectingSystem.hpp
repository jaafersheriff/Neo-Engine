#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "MouseRayComponent.hpp"
#include "SelectableComponent.hpp"
#include "SelectedComponent.hpp"

using namespace neo;

class SelectingSystem : public System {

public:
    SelectingSystem(int maxMarches = 100, float maxDist = 100.f) :
        System("Selecting System"),
        mMaxMarches(maxMarches),
        mMaxDist(maxDist)
    {}


    virtual void update(const float dt) override {
        auto mouseRay = Engine::getSingleComponent<MouseRayComponent>();
        if (!mouseRay || !Mouse::isDown(GLFW_MOUSE_BUTTON_1)) {
            return;
        }

        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            if (auto bb = selectable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                // ray march
                int maxmarches = 100;
                float maxdist = 100.f;
                if (auto camera = Engine::getSingleComponent<CameraComponent>()) {
                    maxdist = camera->getNearFar().y;
                }
                for (int i = 0; i < 20; i++) {
                    if (bb->intersect(mouseRay->position + mouseRay->ray * (maxdist / (float)maxmarches) * (float)i)) {
                        if (auto selected = Engine::getSingleComponent<SelectedComponent>()) {
                            Engine::removeComponent<SelectedComponent>(*selected);
                        }
                        Engine::addComponent<SelectedComponent>(&selectable->getGameObject());
                        return;
                    }
                }
            }
        }
    }

private:
    int mMaxMarches;
    float mMaxDist;
};
