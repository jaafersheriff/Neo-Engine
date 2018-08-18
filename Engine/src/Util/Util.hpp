#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace neo {

    struct Util {

        static float PI() { return glm::pi<float>(); }


        /* Generate a random float [0, 1] */
        static inline float genRandom() {
            return rand() / (float) RAND_MAX;
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

        static char *textFileRead(const char *fn) {
            FILE *fp;
            char *content = NULL;
            int count = 0;
            if (fn != NULL) {
                fp = fopen(fn, "rt");
                if (fp != NULL) {
                    fseek(fp, 0, SEEK_END);
                    count = (int)ftell(fp);
                    rewind(fp);
                    if (count > 0) {
                        content = (char *)malloc(sizeof(char) * (count + 1));
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

        static int textFileWrite(const char *fn, char *s) {
            FILE *fp;
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
    };
}