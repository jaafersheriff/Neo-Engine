#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace FrustaFitting {
    class MockCameraComponent : public Component {
    public:
        MockCameraComponent(GameObject* go) :
            Component(go)
        {}

    };
}