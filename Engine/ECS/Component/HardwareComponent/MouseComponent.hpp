#pragma once

#include "ECS/Component/Component.hpp"
#include "Hardware/Mouse.hpp"

#include <imgui/imgui.h>

namespace neo {

    struct MouseComponent : public Component {
        MouseComponent(Mouse& engineMouse)
            : mFrameMouse(engineMouse)
        {}

        virtual void imGuiEditor() override {
            ImGui::Text("Position: [%0.2f, %0.2f]", mFrameMouse.getPos().x, mFrameMouse.getPos().y);
            ImGui::Text("Speed:    [%0.2f, %0.2f, %0.2f]", mFrameMouse.getSpeed().x, mFrameMouse.getSpeed().y, mFrameMouse.getScrollSpeed());
        }

        const Mouse mFrameMouse;
    };
}