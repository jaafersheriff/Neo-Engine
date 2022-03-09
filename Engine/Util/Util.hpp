#pragma once

#include "Util/Log/Log.hpp"

#include <vector>

#include "glm/gtc/constants.hpp"
#include "glm/glm.hpp"

namespace neo {

#define NEO_UNUSED(...) __noop(__VA_ARGS__)
 
#ifndef NEO_DEBUG_ASSERT
	#ifdef DEBUG_MODE
		#define NEO_ASSERT(c, fmt, ...) \
			if (!(c)) { \
                char buf[1024]; \
                sprintf(buf, fmt, __VA_ARGS__); \
				NEO_LOG_S(neo::util::LogSeverity::Error, "%s: (%s) in %s, file %s on line %d.\n", buf, #c, __func__, __FILE__, __LINE__); \
                abort(); \
			} 
		#define NEO_FAIL(fmt, ...) NEO_ASSERT(false, fmt, __VA_ARGS__)
	#else
		#define NEO_ASSERT(c, fmt, ...) (void)(c); NEO_UNUSED(fmt); NEO_UNUSED(__VA_ARGS__)
		#define NEO_FAIL(fmt, ...) NEO_UNUSED(fmt); NEO_UNUSED(__VA_ARGS__) ; abort()
	#endif // NEO_CONFIG_DEBUG
#endif // NEO_DEBUG_ASSERT

#define NEO_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!

    namespace util {

        static const float PI = glm::pi<float>();

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

        static inline bool fileExists(const char* fn) {
            FILE* fp;
            if (fn != NULL) {
                fp = fopen(fn, "rt");
                if (fp != NULL) {
                    fclose(fp);
                    return true;
                }
            }
            return false;
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
                        content = (char*)malloc(sizeof(char) * (count + 1));
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
    }
}