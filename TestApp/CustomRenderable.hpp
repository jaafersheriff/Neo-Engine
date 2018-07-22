#pragma once

#include "Component/Component.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class CustomRenderable : public Component {
        public:
            CustomRenderable(GameObject &go, Mesh *m) :
                Component(go),
                mesh(m)
            {}

            virtual void init() override {
            }

            virtual void update(float dt) override {
                gameObject->getSpatial()->rotate(glm::mat3(glm::rotate(glm::mat4(1.f), dt, glm::vec3(0, 1, 0))));
            }

            Mesh *mesh;
    };
}