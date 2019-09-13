#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"

#include <glm/glm.hpp>

using namespace neo;

class MouseRayComponent : public Component {
    public:
        glm::vec3 position;
        glm::vec3 ray;
        MouseRayComponent(GameObject *go) :
            Component(go),
            position(0.f),
            ray(0.f)
        {}
};