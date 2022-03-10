#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

namespace neo {
    class Mesh;

    namespace renderable {

        struct PhongRenderable : public Component {
            // These will be replaced by renderer handles eventually
            Material mMaterial;
            Texture* mDiffuseMap;

            PhongRenderable(Texture* diffuseMap, Material material = Material{}) :
                mDiffuseMap(diffuseMap),
                mMaterial(material)
            {}

            virtual std::string getName() const override {
                return "PhongRenderable";
            }

        };
    }
}