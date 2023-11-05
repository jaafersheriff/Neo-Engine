#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

using namespace neo;

namespace Deferred {
    struct GBufferComponent : public Component {
        Material mMaterial;
        Texture* mDiffuseMap;

        GBufferComponent(Texture* diffuseMap, Material material)
            : mDiffuseMap(diffuseMap)
            , mMaterial(material)
        {}

    };
}