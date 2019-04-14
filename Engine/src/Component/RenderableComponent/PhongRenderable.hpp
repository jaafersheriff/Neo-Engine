#pragma once

#include "Component/Component.hpp"

namespace neo {

    namespace renderable {

        class PhongRenderable : public Component {
        public:
            PhongRenderable(GameObject *go) :
                Component(go)
            {}

        };
    }
}