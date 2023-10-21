#if 0
#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        struct PhongShadowRenderable : public Component {
            Texture* mDiffuseMap;
            Material mMaterial;

            PhongShadowRenderable(Texture* texture, Material material) :
                mDiffuseMap(texture),
                mMaterial(material)
            {}

            virtual std::string getName() const override {
                return "PhongShadowRenderable";
            }

            virtual void imGuiEditor() override {
                ImGui::ColorEdit3("Diffuse", &mMaterial.mDiffuse[0]);
            }

        };
    }
}
#endif