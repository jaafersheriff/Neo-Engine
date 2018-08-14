#pragma once

#include "Component/ModelComponent/RenderableComponent.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

class RefractionRenderable : public RenderableComponent {

    public:

        RefractionRenderable(GameObject *go, Mesh *m, float r = 0.5f) :
            RenderableComponent(go, m),
            ratio(r)
        {}

        virtual void update(float dt) override {
            rotation += dt;
            gameObject->getSpatial()->setOrientation(glm::mat3(glm::rotate(glm::mat4(1.f), rotation, glm::vec3(0,1,0))));
        }

        float ratio;

    private:

        float rotation = 0.f;
};