#pragma once

#include "ECS/Component/Component.hpp"

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