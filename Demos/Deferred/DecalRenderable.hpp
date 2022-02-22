#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace Deferred {
    class DecalRenderable : public Component {
    public:
        const Texture& mDiffuseMap;

        DecalRenderable(GameObject* go, const Texture& diffuseMap) :
            Component(go),
            mDiffuseMap(diffuseMap)
        {}

    };
}