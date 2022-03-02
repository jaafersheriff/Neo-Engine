#pragma once

#include "ECS/Component/Component.hpp"

#include "glm/glm.hpp"

namespace neo {

    namespace renderable {

        class OutlineRenderable : public Component {
        public:
            OutlineRenderable(GameObject* go, glm::vec4 color = glm::vec4(1.f, 0.f, 0.f, 1.f), float scale = 3.f)
                : Component(go)
                , mColor(color)
                , mScale(scale)
            {}

            virtual void imGuiEditor() override {
                ImGui::ColorEdit4("Color", &mColor[0]);
                ImGui::SliderFloat("Scale", &mScale, 0.5f, 10.f);
            }

            glm::vec4 mColor;
            float mScale;
        };
    }
}