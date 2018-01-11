#pragma once
#ifndef _AABB_HPP_
#define _AABB_HPP_

#include "Model/Mesh.hpp"
#include "glm/glm.hpp"

class AABB {
    public:
        AABB();
        AABB(Mesh *);
        AABB(const glm::vec3, const glm::vec3);

        glm::vec3 min;
        glm::vec3 max;

        bool intersect(const AABB &);
};

#endif