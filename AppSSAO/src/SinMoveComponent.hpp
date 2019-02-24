#pragma once

#include "Component/Component.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"

#include <glm/glm.hpp>

using namespace neo;

class SinMoveComponent : public Component {

    public:
        SinMoveComponent(GameObject *go, float offset, float base) :
            Component(go),
            offset(offset),
            baseY(base)
        {}

        float offset;
        float baseY;
        float c = offset*rand();

        virtual void update(float dt) override {
            c += dt;    // could also use neoengine::getcurrtime
            glm::vec3 oldPos = mGameObject->getSpatial()->getPosition();
            oldPos.y = baseY + glm::cos(c) * offset;
            mGameObject->getSpatial()->setPosition(oldPos);
        }
};