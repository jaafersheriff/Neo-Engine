#pragma once

#include "Component/Component.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace neo;

class RotationComponent : public Component {

    public:
        RotationComponent(GameObject *go, glm::vec3 speed) :
            Component(go),
            mSpeed(speed)
        {}

        glm::vec3 mSpeed;

};