#pragma once
#ifndef _AABB_HPP_
#define _AABB_HPP_

#include "glm/glm.hpp"

class AABB {
    public:
        AABB();
        AABB(Mesh *);

        glm::vec3 min;
        glm::vec3 max;

        glm::vec3 worldMin;
        glm::vec3 worldMax;

        void update(const glm::mat4 *);

        bool intersect(const AABB &);
};

#endif