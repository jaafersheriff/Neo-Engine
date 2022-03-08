#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/GameObject.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ext/imgui/imgui.h"

namespace neo {

    class RotationComponent : public Component {

    public:
        RotationComponent(GameObject *go, glm::vec3 speed) :
            Component(go),
            mSpeed(speed)
        {}

        virtual void imGuiEditor() override {
            ImGui::SliderFloat3("Speed", &mSpeed[0], -5.f, 5.f);
        }

        glm::vec3 mSpeed;

    };

}