#pragma once

#include "Component/Component.hpp"

namespace neo {

    namespace renderable {

        class WireframeRenderable: public Component {
        public:
            WireframeRenderable(GameObject *go) :
                Component(go)
            {}

        };
    }
}