#pragma once

#include <NeoEngine.hpp>
#include "System/System.hpp"

#include "CustomRenderable.hpp" 

using namespace neo;

class CustomSystem : public System {

    public:
        CustomSystem(CameraControllerComponent *cc) :
            camera(cc) {
            renderables = &NeoEngine::getComponents<CustomRenderable>();
        }

        virtual void update(float dt) override {
            camera->update(dt);
            if (!active) {
                return;
            }

            for (auto r : *renderables) {
                r->update(dt);
            }
        }

        bool active = true;
        const std::vector<CustomRenderable *> *renderables;
        CameraControllerComponent *camera;
};
