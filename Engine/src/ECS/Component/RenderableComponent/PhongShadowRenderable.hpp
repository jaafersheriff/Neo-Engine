#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    namespace renderable {

        class PhongShadowRenderable : public Component {
        public:
            PhongShadowRenderable(GameObject *go) :
                Component(go)
            {}

        };
    }
}