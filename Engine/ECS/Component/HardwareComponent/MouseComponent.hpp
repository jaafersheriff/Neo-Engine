#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"
#include "Hardware/Mouse.hpp"

#include <imgui/imgui.h>

namespace neo {

    class MouseComponent : public Component {

    public:
        MouseComponent (GameObject *go, Mouse& engineMouse)
            : Component(go)
            , mFrameMouse(engineMouse)
        {}

        virtual void imGuiEditor() override {
            ImGui::Text("Position: [%0.2f, %0.2f]", mFrameMouse.getPos().x, mFrameMouse.getPos().y);
            ImGui::Text("Speed:    [%0.2f, %0.2f, %0.2f]", mFrameMouse.getSpeed().x, mFrameMouse.getSpeed().y, mFrameMouse.getScrollSpeed());
        }

        const Mouse mFrameMouse;
    };
}