#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ext/imgui/imgui.h"

namespace neo {

    struct RotationComponent : public Component {
        glm::vec3 mSpeed;

        virtual std::string getName() override {
            return "RotationComponent";
        }

        virtual void imGuiEditor() override {
            ImGui::SliderFloat3("Speed", &mSpeed[0], -5.f, 5.f);
        }

    };
}