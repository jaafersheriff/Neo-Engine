#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

using namespace neo;

class GBufferComponent : public Component {
public:
    GBufferComponent(GameObject* go, const Texture& diffuseMap, Material material) :
        Component(go),
        mDiffuseMap(diffuseMap),
        mMaterial(material)
    {}

    Material mMaterial;
    const Texture& mDiffuseMap;
};