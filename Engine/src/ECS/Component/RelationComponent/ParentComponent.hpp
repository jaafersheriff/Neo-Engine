#pragma once

#include "ECS/Component/Component.hpp"

#include <vector>

namespace neo {

        class ParentComponent : public Component {
        public:
            std::vector<GameObject*> childrenObjects;
            ParentComponent(GameObject *go) :
                Component(go)
            {}

        };
}