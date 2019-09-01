#pragma once

namespace neo {

    class FrustumPlanesComponent : public Component {
        public:
            FrustumPlanesComponent(GameObject *go) :
                Component(go)
            {}

            glm::vec4 mLeft;
            glm::vec4 mRight;
            glm::vec4 mTop;
            glm::vec4 mBottom;
            glm::vec4 mNear;
            glm::vec4 mFar;

            bool isInFrustum(const glm::vec3 position, const float radius) {
                if (_distanceToPlane(mLeft, position) < radius ||
                    _distanceToPlane(mRight, position) < radius ||
                    _distanceToPlane(mBottom, position) < radius ||
                    _distanceToPlane(mTop, position) < radius ||
                    _distanceToPlane(mNear, position) < radius ||
                    _distanceToPlane(mFar, position) < radius) {
                    return false;
                }

                return true;
            }

        private:

            float _distanceToPlane(glm::vec4 plane, glm::vec3 position) {
                return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
            }

    };
}