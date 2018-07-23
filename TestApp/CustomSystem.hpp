#pragma once

#include <NeoEngine.hpp>
#include "System/System.hpp"

#include "CustomComponent.hpp" 

using namespace neo;

class CustomSystem : public System {

    public:
        CustomSystem(CameraControllerComponent *cc) :
            camera(cc) {
            comps = &NeoEngine::getComponents<CustomComponent>();
        }

        const std::vector<CustomComponent *> *comps;
        CameraControllerComponent *camera;

        virtual std::string name() { return "Custom System"; }

        virtual void update(float dt) override {
            camera->update(dt);

            for (auto c : *comps) {
                c->update(dt);
            }
        }

};
