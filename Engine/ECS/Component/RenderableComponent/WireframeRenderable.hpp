#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    namespace renderable {

        struct WireframeRenderable : public Component {
            glm::vec3 mColor = glm::vec3(1.f);

            virtual std::string getName() const override {
                return "WireframeRenderable";
            }

		    virtual void imGuiEditor() override {
                ImGui::ColorEdit3("Color", &mColor[0]);
            };
        };
    }
}