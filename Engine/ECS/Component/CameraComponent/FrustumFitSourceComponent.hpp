#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct FrustumFitSourceComponent : public Component {
        FrustumFitSourceComponent()
        {}

        virtual std::string getName() const override { return "FrustumFitSourceComponent"; }

    };
}