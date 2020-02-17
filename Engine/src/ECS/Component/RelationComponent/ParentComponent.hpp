#pragma once

#include "ECS/Component/Component.hpp"

#include <vector>

namespace neo {

        class ParentComponent : public Component {
        public:
            ParentComponent(GameObject *go) :
                Component(go)
            {}

        };
}