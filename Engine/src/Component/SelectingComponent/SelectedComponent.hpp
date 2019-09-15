#pragma once

#include "Component/Component.hpp"

namespace neo {

    class SelectedComponent : public Component {
        public:
            SelectedComponent(GameObject *go) :
                Component(go)
            {}
        };

}