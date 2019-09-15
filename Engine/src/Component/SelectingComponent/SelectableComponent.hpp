#pragma once

#include "Component/Component.hpp"

namespace neo {

    class SelectableComponent : public Component {
        public:
            SelectableComponent(GameObject *go) :
                Component(go)
            {}
        };

}