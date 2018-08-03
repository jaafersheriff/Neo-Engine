#pragma once

#include <NeoEngine.hpp>
#include "System/System.hpp"

using namespace neo;

class CameraSystem : public System {

    public:
        CameraSystem(CameraControllerComponent *cc) :
            System("Custom System"),
            camera(cc) {
        }

        CameraControllerComponent *camera;

        virtual void update(float dt) override {
            camera->update(dt);
        }

};