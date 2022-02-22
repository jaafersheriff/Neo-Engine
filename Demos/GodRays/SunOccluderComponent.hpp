#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

using namespace neo;

namespace GodRays {
    class SunOccluderComponent : public Component {

    public:
        const Texture& mAlphaMap;
        SunOccluderComponent(GameObject* go, const Texture& texture) :
            Component(go),
            mAlphaMap(texture)
        {}
    };
}
