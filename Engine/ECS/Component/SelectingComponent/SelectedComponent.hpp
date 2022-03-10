#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct SelectedComponent : public Component {

        virtual std::string getName() const override {
            return "SelectedComponent";
        }
    };
}
