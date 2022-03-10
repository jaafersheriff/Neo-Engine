#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct TagComponent : public Component {
        TagComponent(std::string name)
            : mTag(name)
        {}

        virtual std::string getName() const override {
            return "Tag Component";
        }

        std::string mTag;
    };
}
