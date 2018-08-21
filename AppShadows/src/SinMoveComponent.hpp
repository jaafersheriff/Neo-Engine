#pragma once

#include "Component/Component.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"

#include <glm/glm.hpp>

using namespace neo;

class SinMoveComponent : public Component {

    public:
        SinMoveComponent(GameObject *go, glm::vec3 min, glm::vec3 max) :
            Component(go),
            minPos(min),
            maxPos(max)
        {}

        glm::vec3 minPos;
        glm::vec3 maxPos;
        float c = 0.f;

        virtual void update(float dt) override {
            c += dt;    // could also use neoengine::getcurrtime
            float val = (glm::sin(c) + 1.f) / 2.f;
            glm::vec3 newPos = val * (maxPos - minPos) + minPos;
            gameObject->getSpatial()->setPosition(newPos);
        }
};