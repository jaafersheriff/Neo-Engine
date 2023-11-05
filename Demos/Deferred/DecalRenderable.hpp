#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace Deferred {
    class DecalRenderable : public Component {
    public:
        Texture* mDiffuseMap;

        DecalRenderable(Texture* diffuseMap) :
            mDiffuseMap(diffuseMap)
        {}

    };
}