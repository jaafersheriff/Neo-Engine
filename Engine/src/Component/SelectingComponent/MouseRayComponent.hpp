#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"

#include <glm/glm.hpp>

namespace neo {

    class MouseRayComponent : public Component {
    public:
        glm::vec3 position;
        glm::vec3 direction;
        MouseRayComponent(GameObject *go) :
            Component(go),
            position(0.f),
            direction(0.f)
        {}
    };

}