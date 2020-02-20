#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

namespace neo {
    class Mesh;

    namespace renderable {

        class PhongRenderable : public Component {

        public:
            // These will be replaced by renderer handles eventually
            Material mMaterial;
            const Texture& mDiffuseMap;

            PhongRenderable(GameObject *go, const Texture& diffuseMap, Material material) :
                Component(go),
                mDiffuseMap(diffuseMap),
                mMaterial(material)
            {}

        };
    }
}