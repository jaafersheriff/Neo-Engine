#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        struct PhongShadowRenderable : public Component {
            const Texture& mDiffuseMap;
            Material mMaterial;

            PhongShadowRenderable(const Texture& texture, Material material) :
                mDiffuseMap(texture),
                mMaterial(material)
            {}

            virtual std::string getName() override {
                return "PhongShadowRenderable";
            }

            virtual void imGuiEditor() override {
                ImGui::ColorEdit3("Diffuse", &mMaterial.mDiffuse[0]);
            }

        };
    }
}