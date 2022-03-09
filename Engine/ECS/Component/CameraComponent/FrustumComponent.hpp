#pragma once

#include "ECS/Component/Component.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    struct FrustumComponent : public Component {
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
            glm::vec3 position = spatial.getPosition() + box.getCenter();
            glm::vec3 _scale = spatial.getOrientation() * spatial.getScale();
            float scale = std::max(_scale.x, std::max(_scale.y, _scale.z)) * box.getRadius();
            return isInFrustum(position, scale);
        }

        bool isInFrustum(const glm::vec3 position, const float radius) const {
            return _distanceToPlane(mLeft, position) > -radius &&
                _distanceToPlane(mRight, position) > -radius &&
                _distanceToPlane(mBottom, position) > -radius &&
                _distanceToPlane(mTop, position) > -radius &&
                _distanceToPlane(mNear, position) > -radius &&
                _distanceToPlane(mFar, position) > -radius;
        }

    private:

        float _distanceToPlane(glm::vec4 plane, glm::vec3 position) const {
            return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
        }

    };
}
