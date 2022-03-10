#pragma once

#include "ECS/Component/Component.hpp"

namespace neo
{
    struct ShadowCameraComponent : public Component {
        ShadowCameraComponent()
        {}

        virtual std::string getName() const override { return "ShadowCameraComponent"; }
    };
}