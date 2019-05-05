#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"

#include <glm/glm.hpp>

class SnowComponent : public neo::Component{

    public:
        
        SnowComponent(neo::GameObject *gameObject) :
            neo::Component(gameObject),
            snowAngle(0.f, 1.f, 0.f),
            snowSize(0.36f),
            snowColor(0.36f, 0.48f, 0.56f),
            height(0.07f),
            rimColor(1.f),
            rimPower(0.25f)
        {}

        glm::vec3 snowAngle;
        float snowSize;
        glm::vec3 snowColor;
        float height;
        glm::vec3 rimColor;
        float rimPower;
};

