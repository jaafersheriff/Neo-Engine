#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct LightComponent : public Component {
        glm::vec3 mColor;
        glm::vec3 mAttenuation;

        LightComponent(const glm::vec3& col = glm::vec3(1.f), const glm::vec3 att = glm::vec3(0.f)) :
            mColor(col),
            mAttenuation(att)
        {}

        virtual std::string getName() const override { return "LightComponent"; }
        virtual void imGuiEditor() {
            ImGui::SliderFloat3("Color", &mColor[0], 0.f, 1.f);
            ImGui::SliderFloat("Attenuation.x", &mAttenuation[0], 0.f, 1.f);
            ImGui::SliderFloat("Attenuation.y", &mAttenuation[1], 0.f, 0.1f);
            ImGui::SliderFloat("Attenuation.z", &mAttenuation[2], 0.f, 0.01f);
        }
    };
}