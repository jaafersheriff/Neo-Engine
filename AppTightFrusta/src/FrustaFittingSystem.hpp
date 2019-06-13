#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

class FrustaFittingSystem : public System {

public:
    FrustaFittingSystem() :
        System("FrustaFitting System")
    {}

    virtual void update(const float dt) override {
    }
};
