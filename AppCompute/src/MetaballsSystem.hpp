#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include <iostream>

using namespace neo;

class MetaballsSystem : public System {

    public:
        MetaballsSystem() :
            System("Metaballs System")
        {}

        virtual void update(const float dt) override {
        }

};
