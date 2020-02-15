#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

        class ChildComponent : public Component {
        public:
            GameObject* parentObject;
            ChildComponent(GameObject *go, GameObject* parent) :
                Component(go),
                parentObject(parent)
            {}

        };
}