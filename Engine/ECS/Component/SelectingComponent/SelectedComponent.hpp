#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct SelectedComponent : public Component {

        virtual std::string getName() override {
            return "SelectedComponent";
        }
    };
}
