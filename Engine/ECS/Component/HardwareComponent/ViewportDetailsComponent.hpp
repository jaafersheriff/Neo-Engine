#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

#include "Hardware/WindowDetails.hpp"

#include <imgui/imgui.h>

namespace neo {

    class ViewportDetailsComponent : public Component {

    public:
        ViewportDetailsComponent (GameObject *go, glm::uvec2 size, glm::uvec2 pos) 
            : Component(go)
            , mSize(size)
            , mPos(pos)
        {}

        virtual void imGuiEditor() override {
            ImGui::Text("Viewport Size:     [%d, %d]", mSize.x, mSize.y);
			ImGui::Text("Viewport Pos:      [%d, %d]", mPos.x, mPos.y);
        }

        const glm::uvec2 mSize;
        const glm::uvec2 mPos;
    };
}