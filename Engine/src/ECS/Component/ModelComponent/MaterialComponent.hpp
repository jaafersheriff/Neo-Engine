#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

#include <glm/glm.hpp>
#include "ext/imgui/imgui.h"

namespace neo {

    class MaterialComponent : public Component {

        public:

            Material mMaterial;

            MaterialComponent(GameObject *go, Material& material) :
                Component(go),
                mMaterial(material)
            {}

            MaterialComponent(GameObject *go) :
                Component(go)
            {
            }

            virtual void imGuiEditor() override {
                ImGui::SliderFloat3("Ambient", &mMaterial.ambient[0], 0.f, 1.f);
                ImGui::SliderFloat3("Diffuse", &mMaterial.diffuse[0], 0.f, 1.f);
                ImGui::SliderFloat3("Specular", &mMaterial.specular[0], 0.f, 1.f);
                ImGui::SliderFloat("Shine", &mMaterial.shininess, 0.f, 50.f);
            }

    };
}