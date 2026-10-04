#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <cstdlib>

namespace neo {
	namespace util {

		static const float PI = glm::pi<float>();
		static const float EP = static_cast<float>(1e-4);

		/* Generate a random float [0, 1] */
		static inline float genRandom() {
			return rand() / (float)RAND_MAX;
		}

		/* Generate a scaled random value */
		static inline float genRandom(const float val) {
			return genRandom() * val;
		}

		/* Generate a random value in a range [min, max] */
		static inline float genRandom(const float min, const float max) {
			return genRandom() * (max - min) + min;
		}

		/* Generate a random vec3 with values [0, 1] */
		static inline glm::vec3 genRandomVec3() {
			return glm::vec3(genRandom(), genRandom(), genRandom());
		}

		/* Generate random vec3 with values [0, 1] */
		static inline glm::vec3 genRandomVec3(const float min, const float max) {
			return glm::vec3(genRandom(min, max), genRandom(min, max), genRandom(min, max));
		}

		/* Generate random bool */
		static inline bool genRandomBool() {
			return genRandom() < 0.5f;
		}

		static inline float lerp(float a, float b, float t) {
			return a + t * (b - a);
		}

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

		static inline glm::vec3 sphericalToCartesian(const glm::vec3& v) {
			return sphericalToCartesian(v.x, v.y, v.z);
		}
	}
}
