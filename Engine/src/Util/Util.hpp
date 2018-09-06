#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include <vector>

namespace neo {

    struct Util {
        
        static void init() {
            lastFrameTime = getRunTime();
        }

        static void update() {
            /* Update delta time and FPS */
            float runTime = (float)getRunTime();
            totalFrames++;
            timeStep = runTime - lastFrameTime;
            lastFrameTime = runTime;
            nFrames++;
            if (runTime - lastFPSTime >= 1.0) {
                FPS = nFrames;
                if (FPSs.size() == 25) {
                    FPSs.erase(FPSs.begin());
                }
                FPSs.push_back(FPS);
                nFrames = 0;
                lastFPSTime = runTime;
            }

            std::swap(oldPerfs, newPerfs);
            newPerfs.clear();
            oldTotalTimer = newTotalTimer;
            newTotalTimer = 0.0;
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

        static double getRunTime() {
            return glfwGetTime();
        }

        static void startTimer() {
            timer = getRunTime();
        }

        static void endTimer(const std::string &name) {
            double diff = getRunTime() - timer;
            newTotalTimer += diff;
            newPerfs.emplace_back(name, diff);
        }

        /* Timer */
        public:
            static std::vector<std::pair<std::string, double>> oldPerfs;
            static double oldTotalTimer;
        private:
            static double timer;
            static double newTotalTimer;
            static std::vector<std::pair<std::string, double>> newPerfs;
 
        /* FPS*/
        public:
            static std::vector<int> FPSs;
            static int FPS;                 /* Frames per second */
            static double timeStep;         /* Delta time */
            static int totalFrames;         /* Total frames since start up */
        private:
            static double lastFPSTime;      /* Time at which last FPS was calculated */
            static int nFrames;             /* Number of frames in current second */
            static double lastFrameTime;    /* Time at which last frame was rendered */


    };
}