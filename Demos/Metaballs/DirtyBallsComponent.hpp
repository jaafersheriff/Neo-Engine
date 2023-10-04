#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace Metaballs {
    struct DirtyBallsComponent : public Component {
        DirtyBallsComponent() {}

        virtual std::string getName() const override {
            return "DirtyBallsComponent";
        }
    };

}
