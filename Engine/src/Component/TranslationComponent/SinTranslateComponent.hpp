#pragma once

#include "Component/Component.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>

using namespace neo;

class SinTranslateComponent : public Component {

    public:
        SinTranslateComponent(GameObject *go, glm::vec3 offset, glm::vec3 base) :
            Component(go),
            mOffset(offset),
            mBasePosition(base)
        {}

        glm::vec3 mOffset;
        glm::vec3 mBasePosition;
        bool mActive = true;

        virtual void update(float dt) override {
            if (mActive) {
                double time = Util::getRunTime();
                glm::vec3 oldPos = mGameObject->getSpatial()->getPosition();
                oldPos = mBasePosition + (float)glm::cos(time) * mOffset;
                mGameObject->getSpatial()->setPosition(oldPos);
            }
        }
};