#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/Loader.hpp"

#include "Renderer/GLObjects/Mesh.hpp"

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    struct BoundingBoxComponent : public Component {
        virtual std::string getName() const override { return "BoundingBoxComponent"; } 

        glm::vec3 mMin, mMax;

        BoundingBoxComponent() :
            mMin(0.f),
            mMax(0.f) {
        }

        BoundingBoxComponent(std::vector<glm::vec3>& vertices) {
            mMin = glm::vec3(FLT_MAX);
            mMax = glm::vec3(FLT_MIN);

            for (auto vert : vertices) {
                mMin = glm::min(mMin, vert);
                mMax = glm::max(mMax, vert);
            }
        }

        BoundingBoxComponent(MeshData mesh) {
            mMin = mesh.mMin;
            mMax = mesh.mMax;
        }

        float getRadius() const {
            return glm::distance(mMin, mMax) / 2.f;
        }

        glm::vec3 getCenter() const {
            return mMin + ((mMax - mMin) / 2.f);
        }

        bool intersect(const glm::mat4& modelMatrix, glm::vec3 position) const {
            return glm::length(glm::vec3(glm::inverse(modelMatrix) * glm::vec4(position, 1.f))) < getRadius();
        }
    };
}