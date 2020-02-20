#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Renderer/GLObjects/Mesh.hpp"

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class BoundingBoxComponent : public Component {
    public:
        glm::vec3 mMin, mMax;

        BoundingBoxComponent(GameObject *go) :
            Component(go),
            mMin(0.f),
            mMax(0.f) {
        }

        BoundingBoxComponent(GameObject *go, std::vector<glm::vec3>& vertices) :
            Component(go) {
            mMin = glm::vec3(FLT_MAX);
            mMax = glm::vec3(FLT_MIN);

            for (auto vert : vertices) {
                mMin = glm::min(mMin, vert);
                mMax = glm::max(mMax, vert);
            }
        }


        BoundingBoxComponent(GameObject *go, const Mesh& mesh) :
            BoundingBoxComponent(go)
        {
            mMin = mesh.mMin;
            mMax = mesh.mMax;
        }

        float getRadius() const {
            return glm::distance(mMin, mMax) / 2.f;
        }

        bool intersect(const glm::vec3 position) const {
            auto spatial = mGameObject->getComponentByType<SpatialComponent>();
            NEO_ASSERT(spatial, "BoundingBox has no SpatialComponent");
            // TODO - this is broke?
            return glm::length(glm::vec3(glm::inverse(spatial->getModelMatrix()) * glm::vec4(position, 1.f))) < getRadius();
        }
    };
}