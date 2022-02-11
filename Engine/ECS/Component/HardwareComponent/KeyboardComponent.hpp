#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"
#include "Hardware/Keyboard.hpp"

#include <imgui/imgui.h>

namespace neo {

    class KeyboardComponent : public Component {

    public:
        KeyboardComponent(GameObject *go, Keyboard& engineKeyboard)
            : Component(go)
            , mFrameKeyboard(engineKeyboard)
        {}

        virtual void imGuiEditor() override {
        }

        const Keyboard mFrameKeyboard;
    };
}