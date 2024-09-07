#pragma once

#include "Util/Log/Log.hpp"

#include <glm/gtc/constants.hpp>

#include <entt/core/hashed_string.hpp>

#include <fstream>

namespace neo {

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define NEO_UNUSED(...) __noop(__VA_ARGS__)
 
#ifndef NEO_DEBUG_ASSERT
	#ifdef DEBUG_MODE
		#define NEO_ASSERT(c, fmt, ...) \
			if (!(c)) { \
				NEO_LOG_E("ASSERT(%s) in %s, file %s on line %d", #c, __func__, __FILENAME__, __LINE__); \
				NEO_LOG_E(fmt, __VA_ARGS__); \
				__debugbreak(); \
			} 
	#else
		#define NEO_ASSERT(c, fmt, ...) \
			if (!(c)) { \
				NEO_LOG_E("ASSERT(%s) in %s, file %s on line %d", #c, __func__, __FILENAME__, __LINE__); \
				NEO_LOG_E(fmt, __VA_ARGS__); \
			} 
	#endif // NEO_CONFIG_DEBUG
	#define NEO_FAIL(fmt, ...) NEO_ASSERT(false, fmt, __VA_ARGS__)
#endif // NEO_DEBUG_ASSERT

	using HashedString = entt::hashed_string;

	namespace util {

		// https://en.cppreference.com/w/cpp/utility/variant/visit
		template<class... Ts>
		struct VisitOverloaded : Ts... { using Ts::operator()...; };
		template<class... Ts>
		VisitOverloaded(Ts...) -> VisitOverloaded<Ts...>;

		template <class T, class... Ts>
		constexpr auto visit(T&& t, Ts&&... funcs) {
			return std::visit(VisitOverloaded{std::forward<Ts>(funcs)...}, t);
		}

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

		static inline bool fileExists(const char* path) {
			std::ifstream f(path);
			return f.good();
		}

		static inline char* textFileRead(const char* fn) {
			FILE* fp;
			char* content = NULL;
			int count = 0;
			if (fn != NULL) {
				fp = fopen(fn, "rt");
				if (fp != NULL) {
					fseek(fp, 0, SEEK_END);
					count = (int)ftell(fp);
					rewind(fp);
					if (count > 0) {
						content = new char[count + 1];
						count = (int)fread(content, sizeof(char), count, fp);
						content[count] = '\0';
					}
					fclose(fp);
				}
				else {
					printf("error loading %s\n", fn);
				}
			}
			return content;
		}

		static inline int textFileWrite(const char* fn, char* s) {
			FILE* fp;
			int status = 0;
			if (fn != NULL) {
				fopen_s(&fp, fn, "w");
				if (fp != NULL) {
					if (fwrite(s, sizeof(char), strlen(s), fp) == strlen(s)) {
						status = 1;
					}
					fclose(fp);
				}
			}
			return(status);
		}


		static inline time_t getFileModTime(const char* fn) {
			struct stat fileInfo;
			if (stat(fn, &fileInfo) == 0) {
				return fileInfo.st_mtime;
			}
			return 0;
		}
	}
}