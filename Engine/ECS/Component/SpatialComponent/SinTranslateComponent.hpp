#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"


#include "Util/Util.hpp"

#include <glm/glm.hpp>
#include "ext/imgui/imgui.h"

namespace neo {

    struct SinTranslateComponent : public Component {
        glm::vec3 mOffset;
        glm::vec3 mBasePosition;

        virtual std::string getName() override {
            return "SinTranslateComponent";
        }

        virtual void imGuiEditor() override {
            ImGui::SliderFloat3("Offset", &mOffset[0], -10.f, 10.f);
            ImGui::SliderFloat3("Base position", &mBasePosition[0], -100.f, 100.f);
        }

    };
}