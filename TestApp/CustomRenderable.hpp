#pragma once

#include "Component/Component.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class CustomRenderable : public Component {
        public:
            CustomRenderable(GameObject &go, Mesh *m, SpatialComponent *spat = nullptr) :
                Component(go),
                mesh(m),
                spatial(spat)
            {}

            virtual void init() override {
                if (spatial) assert(&spatial->getGameObject() == &getGameObject());
                else assert(spatial = getGameObject().getSpatial());
                update(0.f);
            }

            virtual void update(float dt) override {
                // spatial->rotate(glm::rotate(glm::mat4(spatial->getOrientation()), 5.f, glm::vec3(0, 1, 0)));
            }

            Mesh *mesh;
            SpatialComponent *spatial;
    };
}