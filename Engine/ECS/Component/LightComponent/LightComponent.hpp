#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {

    struct LightComponent : public Component {
        glm::vec3 mColor;

        LightComponent(const glm::vec3& col = glm::vec3(1.f)) :
            mColor(col)
        {}

        virtual std::string getName() const override { return "LightComponent"; }

        virtual void imGuiEditor() {
            ImGui::ColorEdit3("Color", &mColor[0]);
        }
    };
}
