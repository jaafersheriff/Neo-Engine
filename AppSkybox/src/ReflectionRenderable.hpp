#pragma once

#include "Component/RenderableComponent/RenderableComponent.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

class ReflectionRenderable : public RenderableComponent {

    public:

        ReflectionRenderable(GameObject *go, Mesh *m) :
            RenderableComponent(go, m)
        {}

        virtual void update(float dt) override {
            rotation += dt;
            gameObject->getSpatial()->setOrientation(glm::mat3(glm::rotate(glm::mat4(1.f), rotation, glm::vec3(0,1,0))));
        }

    private:
        float rotation = 0.f;
};