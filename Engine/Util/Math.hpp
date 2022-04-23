
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace neo {

	namespace math {
		static const float PI = glm::pi<float>();

		/* Generate a random float [0, 1] */
		static inline float genRandom();

		/* Generate a scaled random value */
		static inline float genRandom(const float val);

		/* Generate a random value in a range [min, max] */
		static inline float genRandom(const float min, const float max);

		/* Generate a random vec3 with values [0, 1] */
		static inline glm::vec3 genRandomVec3();

		/* Generate random vec3 with values [0, 1] */
		static inline glm::vec3 genRandomVec3(const float min, const float max);

		/* Generate random bool */
		static inline bool genRandomBool();

		static inline float lerp(float a, float b, float t);

		// rad is the sphere's radius
		// theta is CCW angle on xy plane
		// phi is angle from +z axis
		// all angles are in radians
		static glm::vec3 sphericalToCartesian(float rad, float theta, float phi);

		static inline glm::vec3 sphericalToCartesian(const glm::vec3& v);

	}
}
