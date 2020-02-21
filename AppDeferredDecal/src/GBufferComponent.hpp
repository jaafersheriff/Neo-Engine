#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

using namespace neo;

class GBufferComponent: public Component {
public:
    Material mMaterial;
    const Texture& mDiffuseMap;

    GBufferComponent(GameObject *go, const Texture& diffuseMap, Material material) :
        Component(go),
        mDiffuseMap(diffuseMap),
        mMaterial(material)
    {}

};