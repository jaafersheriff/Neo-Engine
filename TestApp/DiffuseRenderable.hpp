#pragma once

#include "Component/Component.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class DiffuseRenderable : public Component {
    public:
        DiffuseRenderable(GameObject &go, Mesh *m, glm::vec3 p, float s, glm::vec3 r) :
            Component(go),
            mesh(m),
            position(p),
            scale(s),
            rotation(r)
        {}

        virtual void init() override {
            update(0.f);
        }
        virtual void update(float dt) override {
            M = glm::mat4(1.f);
            M *= glm::translate(glm::mat4(1.f), position);
            M *= glm::scale(glm::mat4(1.f), glm::vec3(scale));
            M *= glm::rotate(glm::mat4(1.f), rotation.x, glm::vec3(1, 0, 0));
            M *= glm::rotate(glm::mat4(1.f), rotation.y, glm::vec3(0, 1, 0));
            M *= glm::rotate(glm::mat4(1.f), rotation.z, glm::vec3(0, 0, 1));
        }

        Mesh *mesh;

        glm::mat4 M;
        glm::vec3 position;
        float scale;
        glm::vec3 rotation;
    };
}