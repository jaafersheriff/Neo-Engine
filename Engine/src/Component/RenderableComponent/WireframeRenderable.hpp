#pragma once

#include "Component/Component.hpp"

namespace neo {

    namespace renderable {

        class WireframeRenderable: public Component {

        public:
            glm::vec3 color;
            WireframeRenderable(GameObject *go, glm::vec3 _color = glm::vec3(1.f)) :
                Component(go),
                color(_color)
            {}

        };
    }
}