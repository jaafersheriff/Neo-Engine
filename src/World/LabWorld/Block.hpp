#pragma once
#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_

#include "Entity/Entity.hpp"
#include "Application/AABB.hpp"

class Block : public Entity {
    public:
        Block(Mesh *, const glm::vec3, const glm::vec3, const glm::vec3, float);
        
        AABB boundingBox;
        bool isHit;
        float velocity;
        
        /* Define model textures for dead and alive game objects*/
        ModelTexture alive = ModelTexture(0.3f, glm::vec3(1.f), glm::vec3(1.f, 1.f, 0.f));
        ModelTexture dead = ModelTexture(0.3f, glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 0.f));

        void update(Entity *, AABB);
};

#endif