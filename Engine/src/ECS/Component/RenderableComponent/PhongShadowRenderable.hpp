#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        class PhongShadowRenderable : public Component {
        public:

            const Texture& mDiffuseMap;
            Material mMaterial;

            PhongShadowRenderable(GameObject *go, const Texture& texture, Material material) :
                Component(go),
                mDiffuseMap(texture),
                mMaterial(material)
            {}

        };
    }
}