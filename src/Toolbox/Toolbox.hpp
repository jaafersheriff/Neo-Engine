#pragma once
#ifndef _TOOLBOX_HPP_
#define _TOOLBOX_HPP_

#include "glm/glm.hpp"
#include <stdlib.h>

class Toolbox {
public:
    static constexpr double PI = 3.14159265359;

    static inline float genRandom() {
        return rand() / (float) RAND_MAX;
    }

    static inline float genRandom(const float min, const float max) {
        return ((float(rand()) / float(RAND_MAX)) * (max - min)) + min;
    }

    static inline glm::vec3 genRandomVec3() {
        return glm::vec3(genRandom(), genRandom(), genRandom());
    }
    
    static inline glm::vec3 genRandomVec3(const float min, const float max) {
        return glm::vec3(genRandom(min, max), genRandom(min, max), genRandom(min, max));
    }
};


#endif