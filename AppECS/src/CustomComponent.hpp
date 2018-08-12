#pragma once

#include "Component/Component.hpp"

#include "GameObject/GameObject.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class CustomComponent : public Component {
        public:
            CustomComponent(GameObject *go) :
                Component(go)
            {}

            virtual void update(float dt) override {
                gameObject->getSpatial()->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), dt, glm::vec3(0, 1, 0))));
            }
    };
}