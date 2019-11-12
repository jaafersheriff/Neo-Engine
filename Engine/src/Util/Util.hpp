#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include <vector>

#include "ext/microprofile.h"

namespace neo {

#define NEO_STR(x) #x
#define NEO_ASSERT(x, s) if (!(x)) { printf("%s: (%s), function %s, file %s, line %d.\n", s, NEO_STR(x), __func__, __FILE__, __LINE__); abort(); }
 
    struct Util {
        
        static void init() {
            mLastFrameTime = getRunTime();
        }

        static void update() {
            MICROPROFILE_SCOPEI("Util", "Util::update", MP_AUTO);
            /* Update delta time and FPS */
            float runTime = (float)getRunTime();
            mTotalFrames++;
            mTimeStep = runTime - mLastFrameTime;
            mLastFrameTime = runTime;
            mFramesInCount++;
            if (runTime - mLastFPSTime >= 1.0) {
                mFPS = mFramesInCount;
                if (mFPSList.size() == 25) {
                    mFPSList.erase(mFPSList.begin());
                }
                mFPSList.push_back(mFPS);
                mFramesInCount = 0;
                mLastFPSTime = runTime;
            }
        }

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

        static glm::vec3 sphericalToCartesian(const glm::vec3 & v) {
            return sphericalToCartesian(v.x, v.y, v.z);
        }

        static bool fileExists(const char *fn) {
            FILE *fp;
            char *content = NULL;
            int count = 0;
            if (fn != NULL) {
                fp = fopen(fn, "rt");
                if (fp != NULL) {
                    fclose(fp);
                    return true;
                }
            }
            return false;
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

        static double getRunTime() {
            return glfwGetTime();
        }

        /* FPS*/
        public:
            static std::vector<int> mFPSList;
            static int mFPS;                 /* Frames per second */
            static double mTimeStep;         /* Delta time */
            static int mTotalFrames;         /* Total frames since start up */
        private:
            static double mLastFPSTime;      /* Time at which last FPS was calculated */
            static int mFramesInCount;       /* Number of frames in current second */
            static double mLastFrameTime;    /* Time at which last frame was rendered */


    };
}