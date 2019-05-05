#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "SurveillanceCamera.hpp"

class SurveillanceCameraSystem : public neo::System {

    public:
        SurveillanceCameraSystem() :
            neo::System("SurveillanceCamera System")
        {}

        virtual void update(const float dt) override {
            for (auto& comp : neo::Engine::getComponents<SurveillanceCamera>()) {
                float scale = comp->getGameObject().getSpatial()->getScale().x;
                glm::vec2 bounds(-scale, scale);
                comp->setOrthoBounds(bounds, bounds);
            }
        }

};