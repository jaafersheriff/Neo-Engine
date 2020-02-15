#pragma once

#include "ECS/Component/Component.hpp"

#include <vector>

namespace neo {

        class ParentComponent : public Component {
        public:
            std::vector<GameObject*> gos;
            ParentComponent(GameObject *go) :
                Component(go)
            {}

        };
}