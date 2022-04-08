#pragma once

#include "ECS/Component/Component.hpp"
#include <imgui/imgui.h>

namespace neo {

    namespace renderable {

        struct WireframeRenderable : public Component {
            WireframeRenderable(glm::vec3 color = glm::vec3(1.f))
                :mColor(color)
            {}

            glm::vec3 mColor;

            virtual std::string getName() const override {
                return "WireframeRenderable";
            }

		    virtual void imGuiEditor() override {
                ImGui::ColorEdit3("Color", &mColor[0]);
            };
        };
    }
}