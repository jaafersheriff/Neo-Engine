#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        class ShadowCasterRenderable : public Component {
        public:
            const Texture& mAlphaMap;

            ShadowCasterRenderable(GameObject *go, const Texture& texture) :
                Component(go),
                mAlphaMap(texture)
            {}

        };
    }
}