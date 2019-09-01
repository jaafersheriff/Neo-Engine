#pragma once

#include "Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    class BoundingBoxComponent : public Component {
        public:
            glm::vec3 mMin, mMax;

            BoundingBoxComponent(GameObject *go) :
                Component(go),
                mMin(FLT_MAX),
                mMax(FLT_MIN)
            {}

            BoundingBoxComponent(GameObject *go, const std::vector<float>& vertices) :
                BoundingBoxComponent(go) {
                for (size_t v = 0; v < vertices.size() / 3; v++) {
                    mMin.x = glm::min(mMin.x, vertices[3 * v + 0]);
                    mMin.y = glm::min(mMin.y, vertices[3 * v + 1]);
                    mMin.z = glm::min(mMin.z, vertices[3 * v + 2]);

                    mMax.x = glm::max(mMax.x, vertices[3 * v + 0]);
                    mMax.y = glm::max(mMax.y, vertices[3 * v + 1]);
                    mMax.z = glm::max(mMax.z, vertices[3 * v + 2]);
                }
            }
    };
}