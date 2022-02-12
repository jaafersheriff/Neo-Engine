#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

#include "Hardware/WindowDetails.hpp"

#include <imgui/imgui.h>

namespace neo {

    class WindowDetailsComponent : public Component {

    public:
        WindowDetailsComponent (GameObject *go, WindowDetails details)
            : Component(go)
            , mDetails(details)
        {}

        virtual void imGuiEditor() override {
            ImGui::Text("True Size:       [%d, %d]", mDetails.getSize().x, mDetails.getSize().y);
            ImGui::Text("Frame Size:      [%d, %d]", mDetails.mFrameSize.x, mDetails.mFrameSize.y);
            ImGui::Text("Fullscreen Size: [%d, %d]", mDetails.mFullscreenSize.x, mDetails.mFullscreenSize.y);
            ImGui::Text("Window Size:     [%d, %d]", mDetails.mWindowSize.x, mDetails.mWindowSize.y);
			ImGui::Text("Window Pos:      [%d, %d]", mDetails.mWindowPos.x, mDetails.mWindowPos.y);
        }

        const WindowDetails mDetails;
    };
}