#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"
#include "Component/AnimationComponent/LineComponent.hpp"

#include <glm/glm.hpp>

using namespace neo;

class SnowComponent : public Component{

    public:
        
        SnowComponent(GameObject *gameObject) :
            Component(gameObject),
            snowAngle(0.f, 1.f, 0.f),
            snowSize(0.36f),
            snowColor(0.36f, 0.48f, 0.56f),
            height(0.07f),
            rimColor(1.f),
            rimPower(0.25f)
        {}

        virtual void update(float dt) override {
            height = -0.19f * snowSize + 0.17f;

            snowAngle = mGameObject->getSpatial()->getV();
            snowAngle.x = -snowAngle.x;
            snowAngle.z = -snowAngle.z;

            // TODO - messaging...
            auto line = mGameObject->getComponentByType<LineComponent>();
            if (line) {
                line->removeNode(1);
                line->addNode(snowAngle);
            }
        }

        glm::vec3 snowAngle;
        float snowSize;
        glm::vec3 snowColor;
        float height;
        glm::vec3 rimColor;
        float rimPower;
};

