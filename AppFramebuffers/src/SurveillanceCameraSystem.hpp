#pragma once

#include "Systems/System.hpp"
#include "NeoEngine.hpp"

using namespace neo;

class SurveillanceCameraSystem : public System {

    public:
        SurveillanceCameraSystem() :
            System("SurveillanceCamera System")
        {}

        virtual void update(const float dt) override {
            for (auto comp : NeoEngine::getComponents<SurveillanceCameraSystem>()) {
                float scale = comp->getGameObject()->getSpatial()->getScale().x;
                glm::vec2 bounds(-scale, scale);
                comp->setOrthoBounds(bounds, bounds);
            }
        }

};