#pragma once

#include "Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    class LightComponent : public Component {
        public:
            glm::vec3 mColor;
            glm::vec3 mAttenuation;

            LightComponent(GameObject *go, const glm::vec3 &col = glm::vec3(1.f), const glm::vec3 att = glm::vec3(1.f)) :
                Component(go),
                mColor(col),
                mAttenuation(att)
            {}

    };
}