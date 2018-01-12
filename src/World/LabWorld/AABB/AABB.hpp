#pragma once
#ifndef _AABB_HPP_
#define _AABB_HPP_

#include "Entity/Entity.hpp"
#include "glm/glm.hpp"

class AABB {
    public:
        AABB();
        AABB(Entity *);

        Entity *entity;
        glm::vec3 min;
        glm::vec3 max;

        glm::vec3 worldMin;
        glm::vec3 worldMax;

        bool intersect(const AABB &);
};

#endif