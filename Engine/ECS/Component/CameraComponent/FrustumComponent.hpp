#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class FrustumComponent : public Component {
        public:
            FrustumComponent(GameObject *go) :
                Component(go)
            {}

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
            bool isInFrustum(const glm::vec3 position, const float radius) {
                return _distanceToPlane(mLeft, position)   > -radius &&
                       _distanceToPlane(mRight, position)  > -radius &&
                       _distanceToPlane(mBottom, position) > -radius &&
                       _distanceToPlane(mTop, position)    > -radius &&
                       _distanceToPlane(mNear, position)   > -radius &&
                       _distanceToPlane(mFar, position)    > -radius;
            }

        private:

            float _distanceToPlane(glm::vec4 plane, glm::vec3 position) {
                return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
            }

    };
}