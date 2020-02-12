#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    namespace renderable {

        class AlphaTestRenderable : public Component {
        public:
            AlphaTestRenderable(GameObject *go) :
                Component(go)
            {}

        };
    }
}