#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include <glm/glm.hpp>

using namespace neo;

class SnowComponent : public Component{

    public:
        
        SnowComponent(GameObject *gameObject) :
            Component(gameObject),
            snowAngle(glm::vec3(0.f, 1.f, 0.f)),
            snowSize(0.36),
            snowColor(0.36, 0.48, 0.56),
            height(0.07),
            rimColor(glm::vec3(1.f)),
            rimPower(0.25)
        {}

        virtual void update(float dt) override {
            snowAngle = gameObject->getSpatial()->getV();
            snowAngle.x = -snowAngle.x;
            snowAngle.z = -snowAngle.z;
        }

        glm::vec3 snowAngle;
        float snowSize;
        glm::vec3 snowColor;
        float height;
        glm::vec3 rimColor;
        float rimPower;
};

