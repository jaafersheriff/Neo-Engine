#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

        class ChildComponent : public Component {
        public:
            GameObject* parent;
            ChildComponent(GameObject *go, GameObject* parent) :
                Component(go),
                parent(parent)
            {}

        };
}