#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class BoundingBoxComponent : public Component {
    public:
        glm::vec3 mMin, mMax;

        BoundingBoxComponent(GameObject *go, const std::vector<float>& vertices) :
            Component(go) {
            assert(vertices.size() > 0);
            for (size_t v = 0; v < vertices.size() / 3; v++) {
                mMin.x = glm::min(mMin.x, vertices[3 * v + 0]);
                mMin.y = glm::min(mMin.y, vertices[3 * v + 1]);
                mMin.z = glm::min(mMin.z, vertices[3 * v + 2]);

                mMax.x = glm::max(mMax.x, vertices[3 * v + 0]);
                mMax.y = glm::max(mMax.y, vertices[3 * v + 1]);
                mMax.z = glm::max(mMax.z, vertices[3 * v + 2]);
            }
        }

        float getRadius() const {
            return glm::distance(mMin, mMax) / 2.f;
        }

        bool intersect(const glm::vec3 position) const {
            auto spatial = mGameObject->getComponentByType<SpatialComponent>();
            assert(spatial);
            // TODO - this is broke
            return glm::length(glm::vec3(glm::inverse(glm::scale(glm::mat4(1.f), spatial->getScale()) * glm::translate(glm::mat4(1.f), spatial->getPosition())) * glm::vec4(position, 1.f))) <= getRadius();
        }
    };
}