#pragma once

#include "ECS/Component/Component.hpp"

#include <glm/glm.hpp>
#include "ext/imgui/imgui.h"

namespace neo {

    struct Material {
        glm::vec3 ambient = glm::vec3(1.f);
        glm::vec3 diffuse = glm::vec3(1.f);
        glm::vec3 specular = glm::vec3(1.f);
        // float transmittance[3];
        // float emission[3];
        float shininess = 1.f;
        // float ior;      // index of refraction
        // float dissolve; // 1 == opaque; 0 == fully transparent
    };

    class MaterialComponent : public Component {

        public:

            Material material;

            MaterialComponent(GameObject *go, const glm::vec3 amb = glm::vec3(0.2f), const glm::vec3 diffuse = glm::vec3(1.f), const glm::vec3 spec = glm::vec3(0.f), const float shine = 1.f) :
                Component(go)
            {
                material.ambient = glm::vec3(amb);
                material.diffuse = diffuse;
                material.specular = spec;
                material.shininess = shine;
            }

            virtual void imGuiEditor() override {
                ImGui::SliderFloat3("Ambient", &material.ambient[0], 0.f, 1.f);
                ImGui::SliderFloat3("Diffuse", &material.diffuse[0], 0.f, 1.f);
                ImGui::SliderFloat3("Specular", &material.specular[0], 0.f, 1.f);
                ImGui::SliderFloat("Shine", &material.shininess, 0.f, 50.f);
            }

    };
}