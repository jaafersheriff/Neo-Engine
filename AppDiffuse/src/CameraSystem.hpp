#pragma once

#include <NeoEngine.hpp>
#include "System/System.hpp"

using namespace neo;

class CameraSystem : public System {

    public:
        CameraSystem() :
            System("Camera System")
        {}

        virtual void update(float dt) override {
            NeoEngine::getComponents<CameraControllerComponent>()[0]->update(dt);
        }

};