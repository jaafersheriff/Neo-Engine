#pragma once
#ifndef _BOUNDING_SPHERE_HPP_
#define _BOUNDING_SPHERE_HPP_

#include "glm/glm.hpp"
#include "Model/Mesh.hpp"

class BoundingSphere {
    public:
        BoundingSphere();
        BoundingSphere(float);
        BoundingSphere(Mesh *);

        glm::vec3 position;
        float radius;

        void update(glm::vec3 &);
        bool intersect(const BoundingSphere &);
};

#endif
