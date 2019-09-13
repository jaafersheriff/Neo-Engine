#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "SelectedComponent.hpp"

using namespace neo;

class SelectableCameraControllerSystem : public CameraControllerSystem {

public:
    SelectableCameraControllerSystem() :
        CameraControllerSystem()
    {}


    virtual void update(const float dt) override {
        if (auto comp = Engine::getSingleComponent<CameraControllerComponent>()) {
            _updatePosition(dt, *comp);
            // if (!Engine::getSingleComponent<SelectedComponent>()) {
                _updateLook(dt, *comp);
            // }
        }
    }
};
