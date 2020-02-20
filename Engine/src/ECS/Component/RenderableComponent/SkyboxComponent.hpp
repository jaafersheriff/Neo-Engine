#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
    namespace renderable {
        class SkyboxComponent : public Component {
        public:
            const Texture& mCubeMap;
            SkyboxComponent(GameObject *go, const Texture& cubeMap) :
                Component(go),
                mCubeMap(cubeMap)
            {}
        };
    }
}