/* Base billboard class
 * Contains simplest structures to render a billboard */
#pragma once
#ifndef _BILLBOARD_HPP_
#define _BILLBOARD_HPP_

#include "Model/ModelTexture.hpp"
#include "Model/Mesh.hpp"

#include "glm/glm.hpp"

class Billboard {
    public: 
        /* Center of billboard */
        glm::vec3 center;

        /* 2-D size of billboard */
        glm::vec2 size;

        /* References to billboard mesh and texture */
        Mesh *mesh;
        Texture texture;

        /* Constructors */
        Billboard(const glm::vec3, const glm::vec2);
        Billboard(Texture, const glm::vec3, const glm::vec2);
};

#endif
