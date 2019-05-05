#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "SurveillanceCamera.hpp"

using namespace neo;

class SurveillanceCameraSystem : public System {

    public:
        SurveillanceCameraSystem() :
            System("SurveillanceCamera System")
        {}

        virtual void update(const float dt) override {
            for (auto& comp : Engine::getComponents<SurveillanceCamera>()) {
                float scale = comp->getGameObject().getSpatial()->getScale().x;
                glm::vec2 bounds(-scale, scale);
                comp->setOrthoBounds(bounds, bounds);
            }
        }

};