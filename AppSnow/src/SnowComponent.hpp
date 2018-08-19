#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include <glm/glm.hpp>

using namespace neo;

class SnowComponent : public Component{

    public:
        
        SnowComponent(GameObject *gameObject) :
            Component(gameObject) {
        }

        virtual void update(float dt) override {
            snowAngle = gameObject->getSpatial()->getV();
        }

        glm::vec3 snowAngle;
        float snowSize = 0.36f;
        glm::vec3 snowColor = glm::vec3(0.39f, 0.6f, 0.7f);
        float height = 0.07f;
        glm::vec3 rimColor = glm::vec3(1.f);
        float rimPower = 0.373f;
};

