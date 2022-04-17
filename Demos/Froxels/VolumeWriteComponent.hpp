#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace Froxels {
    struct VolumeWriteComponent : public Component {
        VolumeWriteComponent()
        {}

        virtual std::string getName() const override { return "VolumeWriteComponent"; }
    };
}