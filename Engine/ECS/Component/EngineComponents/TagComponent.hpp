#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

namespace neo {

    class TagComponent : public Component {

    public:
        TagComponent(GameObject *go, std::string name)
            , mTag(name)
        {}

        const std::string mTag;
    };
}
