/* Base billboard class
 * Contains simplest structures to render a billboard */
#pragma once
#ifndef _BILLBOARD_HPP_
#define _BILLBOARD_HPP_

#include "Model/ModelTexture.hpp"
#include "Model/Mesh.hpp"
#include "Camera/Camera.hpp"

#include "glm/glm.hpp"

class Billboard {
    public: 
        glm::vec3 center;       /* Center of billboard */
        glm::vec2 size;         /* 2-D size of billboard */
        float distance = 0.f;   /* Distance to camera */
        float rotation = 0.f;   /* Billboard rotation */

        /* References to billboard mesh and texture */
        Mesh *mesh;
        Texture *texture;

        /* Constructors */
        Billboard(const glm::vec3, const glm::vec2);
        Billboard(Texture *, const glm::vec3, const glm::vec2);

        /* Update distance to camera */
        void update(const Camera &);
};

#endif
