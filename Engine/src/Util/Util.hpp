#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace neo {

    struct Util {

        static float PI() { return glm::pi<float>(); }

        // rad is the sphere's radius
        // theta is CCW angle on xy plane
        // phi is angle from +z axis
        // all angles are in radians
        static glm::vec3 sphericalToCartesian(float rad, float theta, float phi) {
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            return glm::vec3(
                rad * sinPhi * cosTheta,
                rad * sinPhi * sinTheta,
                rad * cosPhi
            );
        }

        static glm::vec3 sphericalToCartesian(const glm::vec3 & v) {
            return sphericalToCartesian(v.x, v.y, v.z);
        }

    };
}