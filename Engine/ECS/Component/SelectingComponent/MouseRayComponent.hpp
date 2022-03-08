#pragma once

#include "ECS/Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    struct MouseRayComponent : public Component {
        glm::vec3 mPosition;
        glm::vec3 mDirection;

        virtual std::string getName() override {
            return "MouseRayComponent";
        }
    };
}
