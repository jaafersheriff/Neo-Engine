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

        virtual void update(float dt) override {
            glm::mat4 R(1.f);
            R *= glm::rotate(glm::mat4(1.f), dt * mSpeed.x, glm::vec3(1, 0, 0));
            R *= glm::rotate(glm::mat4(1.f), dt * mSpeed.y, glm::vec3(0, 1, 0));
            R *= glm::rotate(glm::mat4(1.f), dt * mSpeed.z, glm::vec3(0, 0, 1));
            this->getGameObject().getSpatial()->rotate(glm::mat3(R));
        }
};