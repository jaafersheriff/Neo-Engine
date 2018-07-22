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

        virtual std::string name() { return "Custom System"; }

        virtual void update(float dt) override {
            camera->update(dt);

            for (auto r : *renderables) {
                r->update(dt);
            }
        }

        const std::vector<CustomRenderable *> *renderables;
        CameraControllerComponent *camera;
};
