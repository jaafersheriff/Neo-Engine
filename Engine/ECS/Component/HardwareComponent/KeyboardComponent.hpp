#pragma once

#include "ECS/Component/Component.hpp"
#include "Hardware/Keyboard.hpp"

#include <imgui/imgui.h>

namespace neo {

    struct KeyboardComponent : public Component {
        KeyboardComponent(Keyboard& engineKeyboard)
            : mFrameKeyboard(engineKeyboard)
        {}

        virtual std::string getName() override {
            return "KeyboardComponent";
        }

        const Keyboard mFrameKeyboard;
    };
}