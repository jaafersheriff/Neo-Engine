#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    namespace renderable {

        class ShadowCasterRenderable : public Component {
        public:
            ShadowCasterRenderable(GameObject *go) :
                Component(go)
            {}

        };
    }
}