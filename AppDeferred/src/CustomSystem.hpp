#pragma once

#include <NeoEngine.hpp>

#include "System/System.hpp"

using namespace neo;

class CustomSystem : public System {

    public:

        CustomSystem() :
            System("Custom System")
        {}

        virtual void update(float dt) override {
            /* Update components */
            for (auto go : NeoEngine::getGameObjects())
                for (auto comp : go->getAllComponents()) {
                    comp->update(dt);
                }
        }
};