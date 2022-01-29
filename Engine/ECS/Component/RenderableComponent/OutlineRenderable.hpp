#pragma once

#include "ECS/Component/Component.hpp"

#include "glm/glm.hpp"

namespace neo {

    namespace renderable {

        class OutlineRenderable : public Component {
        public:
            OutlineRenderable(GameObject *go, glm::vec4 color = glm::vec4(1.f, 0.f, 0.f, 0.4f), float scale = 0.2f) :
                Component(go),
                mColor(color),
                mScale(scale)
            {}

            virtual void imGuiEditor() override {
                ImGui::ColorEdit4("Color", &mColor[0]);
                ImGui::SliderFloat("Scale", &mScale, 0.01f, 1.f);
            }

            glm::vec4 mColor;
            float mScale;
        };
    }
}