#pragma once

#include "Component/Component.hpp"

namespace neo {

    class LightComponent : public Component {
        public:
            LightComponent(GameObject *go, const glm::vec3 &col = glm::vec3(1.f), const glm::vec3 att = glm::vec3(1.f)) :
                Component(go),
                mColor(col),
                mAttenuation(att)
            {}

            /* Getters */
            const glm::vec3 & getColor() const { return mColor; }
            const glm::vec3 & getAttenuation() const { return mAttenuation; }

            /* Setters */
            void setColor(const glm::vec3 col) { this->mColor = col; }
            void setAttenuation(const glm::vec3 att) { this->mAttenuation = att; }
        private:
            glm::vec3 mColor;
            glm::vec3 mAttenuation;
    };
}