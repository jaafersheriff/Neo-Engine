/* Cloud class
 * Derived from Billboard
 * Contains cloud-specific billboard things */
#pragma once
#ifndef _CLOUD_BILLBOARD_HPP_
#define _CLOUD_BILLBOARD_HPP_

#include "Billboard/Billboard.hpp"
#include "Camera/Camera.hpp"

class CloudBillboard : public Billboard {
    public:
        float distance = 0.f;   /* Distance to camera */
        float rotation = 0.f;   /* Billboard's rotation */

        /* Constructors */
        CloudBillboard(const glm::vec3, const glm::vec2);
        CloudBillboard(Texture *, const glm::vec3, const glm::vec2);

        /* Update distance to camera */
        void update(const Camera *);
};

#endif
