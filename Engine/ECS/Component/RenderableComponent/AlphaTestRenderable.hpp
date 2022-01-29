#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        class AlphaTestRenderable : public Component {
        public:
            const Texture& mDiffuseMap;
            AlphaTestRenderable(GameObject *go, const Texture& texture) :
                Component(go),
                mDiffuseMap(texture)
            {}

        };
    }
}