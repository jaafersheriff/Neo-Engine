#pragma once
#ifndef _BOUNDING_BOX_HPP_
#define _BOUNDING_BOX_HPP_d

#include "glm/glm.hpp"
#include "Model/Mesh.hpp"

class BoundingBox {
    public:
        BoundingBox();
        BoundingBox(Mesh *);

        glm::vec3 min;
        glm::vec3 max;

        glm::vec3 worldMin;
        glm::vec3 worldMax;

        void update(glm::mat4 &);

        bool intersect(const BoundingBox &);
};

#endif