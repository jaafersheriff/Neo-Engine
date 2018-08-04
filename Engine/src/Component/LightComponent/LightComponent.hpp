#pragma once

#include "Component/Component.hpp"

namespace neo {

    class LightComponent : public Component {
        public:
            LightComponent(GameObject &go, const glm::vec3 &col = glm::vec3(1.f), const glm::vec3 att = glm::vec3(1.f)) :
                Component(go),
                color(col),
                attenuation(att)
            {}

            /* Getters */
            const glm::vec3 & getColor() const { return color; }
            const glm::vec3 & getAttenuation() const { return attenuation; }

            /* Setters */
            void setColor(const glm::vec3 col) { this->color = col; }
            void setAttenuation(const glm::vec3 att) { this->attenuation = att; }
        private:
            glm::vec3 color;
            glm::vec3 attenuation;
    };
}