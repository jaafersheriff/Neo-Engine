#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {

    namespace renderable {

        struct OutlineRenderable : public Component {
            OutlineRenderable(glm::vec4 color = glm::vec4(1.f, 0.f, 0.f, 1.f), float scale = 3.f)
                : mColor(color)
                , mScale(scale)
            {}

            virtual std::string getName() const override {
                return "OutlineRenderable";
            }

            virtual void imGuiEditor() override {
                ImGui::ColorEdit4("Color", &mColor[0]);
                ImGui::SliderFloat("Scale", &mScale, 0.5f, 10.f);
            }

            glm::vec4 mColor;
            float mScale;
        };
    }
}