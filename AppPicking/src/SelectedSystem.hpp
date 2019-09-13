#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "MouseRayComponent.hpp"
#include "SelectableComponent.hpp"
#include "SelectedComponent.hpp"

using namespace neo;

class SelectedSystem : public System {

public:
    SelectedSystem() :
        System("Selecting System")
    {}


    virtual void update(const float dt) override {
        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            if (auto material = selectable->getGameObject().getComponentByType<MaterialComponent>()) {
                material->mDiffuse = glm::vec3(1.f);
            }
        }

        if (auto selected = Engine::getSingleComponent<SelectedComponent>()) {
            if (auto material = selected->getGameObject().getComponentByType<MaterialComponent>()) {
                material->mDiffuse = glm::vec3(1.f, 0.f, 0.f);
            }
        }
    }

    virtual void imguiEditor() override {

        if (auto selected = Engine::getSingleComponent<SelectedComponent>()) {
            glm::vec3 scale = selected->getGameObject().getSpatial()->getScale();
            ImGui::SliderFloat3("Scale", &scale[0], 0.f, 3.f);
            selected->getGameObject().getSpatial()->setScale(scale);
        }
    }
};
