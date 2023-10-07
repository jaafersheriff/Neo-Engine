#pragma once

#include "ECS/Component/Component.hpp"

#include "Hardware/WindowDetails.hpp"

namespace neo {

    struct ViewportDetailsComponent : public Component {
        ViewportDetailsComponent(glm::uvec2 size, glm::uvec2 pos) 
            : mSize(size)
            , mPos(pos)
        {}

        virtual std::string getName() const override {
            return "ViewportDetailsComponent";
        }

        virtual void imGuiEditor() override {
            ImGui::Text("Viewport Size: [%d, %d]", mSize.x, mSize.y);
			ImGui::Text("Viewport Pos:  [%d, %d]", mPos.x, mPos.y);
        }

        glm::uvec2 mSize;
        glm::uvec2 mPos;
    };
}