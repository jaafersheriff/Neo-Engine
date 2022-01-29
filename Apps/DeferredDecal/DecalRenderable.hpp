#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

class DecalRenderable: public Component {
public:
    const Texture& mDiffuseMap;

    DecalRenderable(GameObject *go, const Texture& diffuseMap) :
        Component(go),
        mDiffuseMap(diffuseMap)
    {}

};