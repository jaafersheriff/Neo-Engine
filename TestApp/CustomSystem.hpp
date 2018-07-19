#pragma once

#include <NeoEngine.hpp>
#include "System/System.hpp"

#include "CustomRenderable.hpp" 

using namespace neo;

class CustomSystem : public System {

    public:
        virtual void update(float dt) override {
            if (!active) {
                return;
            }

            auto renderables = NeoEngine::getComponents<CustomRenderable>();
            for (auto r : renderables) {
                r->update(dt);
            }
        }

        bool active = true;
};
