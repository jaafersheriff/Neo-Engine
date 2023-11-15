
#pragma once

#include "ECS/Component/Component.hpp"
#include "Util/Util.hpp"

#include <string>

namespace neo {

    struct PointLightComponent : public Component {
        PointLightComponent() {}
        
        virtual std::string getName() const override {
            return "PointLightComponent";
        }
    };
}
