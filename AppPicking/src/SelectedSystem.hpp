#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "MouseRayComponent.hpp"
#include "SelectableComponent.hpp"
#include "SelectedComponent.hpp"

#include <functional>

using namespace neo;

class SelectedSystem : public System {

public:
    SelectedSystem(
        std::function<void(SelectableComponent*)> resetOperation,
        std::function<void(SelectedComponent*)> selectOperation) :
        System("Selected System"),
        resetOperation(resetOperation),
        selectOperation(selectOperation)
    {}

    virtual void update(const float dt) override {
        // Operate on unselected objects
        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            resetOperation(selectable);
        }

        // Operate on selected objects
        for (auto selected : Engine::getComponents<SelectedComponent>()) {
            selectOperation(selected);
        }
    }

    virtual void imguiEditor() override {
        if (auto selected = Engine::getSingleComponent<SelectedComponent>()) {
            glm::vec3 scale = selected->getGameObject().getComponentByType<SpatialComponent>()->getScale();
            ImGui::SliderFloat3("Scale", &scale[0], 0.f, 3.f);
            selected->getGameObject().getComponentByType<SpatialComponent>()->setScale(scale);
        }
    }

private:
    std::function<void(SelectableComponent*)> resetOperation;
    std::function<void(SelectedComponent*)> selectOperation;
};
