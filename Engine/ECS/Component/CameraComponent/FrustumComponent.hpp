#pragma once

#include "ECS/Component/Component.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    struct FrustumComponent : public Component {
        virtual std::string getName() const override {
            return "FrustumComponent";
        }

        // Bounds
        glm::vec3 NearLeftBottom{ 0.f, 0.f, 0.f };
        glm::vec3 NearLeftTop{ 0.f, 0.f, 0.f };
        glm::vec3 NearRightBottom{ 0.f, 0.f, 0.f };
        glm::vec3 NearRightTop{ 0.f, 0.f, 0.f };
        glm::vec3 FarLeftBottom{ 0.f, 0.f, 0.f };
        glm::vec3 FarLeftTop{ 0.f, 0.f, 0.f };
        glm::vec3 FarRightBottom{ 0.f, 0.f, 0.f };
        glm::vec3 FarRightTop{ 0.f, 0.f, 0.f };

        // Planes
        glm::vec4 mLeft{ 0.f, 0.f, 0.f, 0.f };
        glm::vec4 mRight{ 0.f, 0.f, 0.f, 0.f };
        glm::vec4 mTop{ 0.f, 0.f, 0.f, 0.f };
        glm::vec4 mBottom{ 0.f, 0.f, 0.f, 0.f };
        glm::vec4 mNear{ 0.f, 0.f, 0.f, 0.f };
        glm::vec4 mFar{ 0.f, 0.f, 0.f, 0.f };

        // Test if an object is inside the frustum
        bool isInFrustum(const SpatialComponent& spatial, const BoundingBoxComponent& box) const {
            glm::mat4 M = spatial.getModelMatrix();
            glm::vec3 min = M * glm::vec4(box.mMin, 1.f);
            glm::vec3 max = M * glm::vec4(box.mMax, 1.f);

            return true
                 && _boxInsideOfPlane(mLeft, min, max)
                 && _boxInsideOfPlane(mRight, min, max)
                 && _boxInsideOfPlane(mTop, min, max)
                 && _boxInsideOfPlane(mBottom, min, max)
                 && _boxInsideOfPlane(mNear, min, max)
                 && _boxInsideOfPlane(mFar, min, max)
            ;
        }

    private:

        inline float _distanceToPlane(glm::vec4 plane, glm::vec3 position) const {
            return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
        }

        // https://iquilezles.org/articles/frustumcorrect/
        inline bool _boxInsideOfPlane(glm::vec4 plane, glm::vec3 min, glm::vec3 max) const {
            return !
                (  glm::dot(plane, glm::vec4(min.x, min.y, min.z, 1.f)) < 0.f
                && glm::dot(plane, glm::vec4(max.x, min.y, min.z, 1.f)) < 0.f
                && glm::dot(plane, glm::vec4(min.x, max.y, min.z, 1.f)) < 0.f
                && glm::dot(plane, glm::vec4(max.x, max.y, min.z, 1.f)) < 0.f
                && glm::dot(plane, glm::vec4(min.x, min.y, max.z, 1.f)) < 0.f
                && glm::dot(plane, glm::vec4(max.x, min.y, max.z, 1.f)) < 0.f
                && glm::dot(plane, glm::vec4(min.x, max.y, max.z, 1.f)) < 0.f
                && glm::dot(plane, glm::vec4(max.x, max.y, max.z, 1.f)) < 0.f
            );

        }

    };
}
