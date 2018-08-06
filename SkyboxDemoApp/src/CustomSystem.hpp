#pragma once

#include <NeoEngine.hpp>
#include "System/System.hpp"

using namespace neo;

class CustomSystem : public System {

    public:
        CustomSystem() :
            System("Camera System")
        {}

        virtual void update(float dt) override {
            NeoEngine::getComponents<CameraControllerComponent>()[0]->update(dt);
            for (auto renderable : NeoEngine::getComponents<RenderableComponent>()) {
                renderable->update(dt);
            }
        }

};