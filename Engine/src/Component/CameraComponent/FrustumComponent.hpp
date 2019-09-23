#pragma once

namespace neo {

    class FrustumComponent : public Component {
        public:
            FrustumComponent(GameObject *go) :
                Component(go)
            {}

            // Bounds
            glm::vec3 NearLeftBottom;
            glm::vec3 NearLeftTop;
            glm::vec3 NearRightBottom;
            glm::vec3 NearRightTop;
            glm::vec3 FarLeftBottom;
            glm::vec3 FarLeftTop;
            glm::vec3 FarRightBottom;
            glm::vec3 FarRightTop;

            // Planes
            glm::vec4 mLeft;
            glm::vec4 mRight;
            glm::vec4 mTop;
            glm::vec4 mBottom;
            glm::vec4 mNear;
            glm::vec4 mFar;

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
