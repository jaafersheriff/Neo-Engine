#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "MouseRayComponent.hpp"
#include "SelectedComponent.hpp"

#include <functional>

using namespace neo;

template <typename CompT>
class SelectedSystem : public System {

public:
    SelectedSystem(
        std::function<void(CompT*)> resetOperation,
        std::function<void(SelectedComponent*)> selectOperation) :
        System("Selected System"),
        resetOperation(resetOperation),
        selectOperation(selectOperation)
    {}

    virtual void update(const float dt) override {
        // Operate on unselected objects
        for (auto selectable : Engine::getComponents<CompT>()) {
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
    std::function<void(CompT*)> resetOperation;
    std::function<void(SelectedComponent*)> selectOperation;
};
