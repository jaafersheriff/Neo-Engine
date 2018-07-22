#pragma once

#include "Component/RenderableComponent/RenderableComponent.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class CustomRenderable : public RenderableComponent {
        public:
            CustomRenderable(GameObject &go, Mesh *m) :
                RenderableComponent(go, m)
            {}

            virtual void update(float dt) override {
                gameObject->getSpatial()->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), dt, glm::vec3(0, 1, 0))));
            }
    };
}