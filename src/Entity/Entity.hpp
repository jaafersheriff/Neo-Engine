/* Base Entity class
 * Contains simplest structures to render geometry */
#pragma once
#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

#include "Model/ModelTexture.hpp"
#include "Model/Mesh.hpp"

#include "glm/glm.hpp"

class Entity {
    public:
        /* Position, rotation, scale */
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        /* References to mesh and texture */
        Mesh *mesh;
        ModelTexture modelTexture;
        
        /* Constructors */
        Entity(const glm::vec3, const glm::vec3, const glm::vec3);
        Entity(Mesh *, const glm::vec3, const glm::vec3, const glm::vec3);
        Entity(Mesh *, ModelTexture, const glm::vec3, const glm::vec3, const glm::vec3);
        Entity(Mesh *, Texture *, const glm::vec3, const glm::vec3, const glm::vec3);

        /* Update */
        virtual void update();
};

#endif
