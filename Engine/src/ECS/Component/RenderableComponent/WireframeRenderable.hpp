#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    namespace renderable {

        class WireframeRenderable: public Component {

        public:
            glm::vec3 mColor;

            WireframeRenderable(GameObject *go, glm::vec3 color = glm::vec3(1.f)) :
                Component(go),
                mColor(color)
            {}

        };
    }
}