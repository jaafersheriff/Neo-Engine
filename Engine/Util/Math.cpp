
#include "Util/Math.hpp"

namespace neo {

	namespace math {

		inline float genRandom() {
			return rand() / (float)RAND_MAX;
		}

		inline float genRandom(const float val) {
			return genRandom() * val;
		}

		inline float genRandom(const float min, const float max) {
			return genRandom() * (max - min) + min;
		}

		inline glm::vec3 genRandomVec3() {
			return glm::vec3(genRandom(), genRandom(), genRandom());
		}

		inline glm::vec3 genRandomVec3(const float min, const float max) {
			return glm::vec3(genRandom(min, max), genRandom(min, max), genRandom(min, max));
		}

		inline bool genRandomBool() {
			return genRandom() < 0.5f;
		}

		inline float lerp(float a, float b, float t) {
			return a + t * (b - a);
		}

		// rad is the sphere's radius
		// theta is CCW angle on xy plane
		// phi is angle from +z axis
		// all angles are in radians
		glm::vec3 sphericalToCartesian(float rad, float theta, float phi) {
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

		inline glm::vec3 sphericalToCartesian(const glm::vec3& v) {
			return sphericalToCartesian(v.x, v.y, v.z);
		}

 
	}
}