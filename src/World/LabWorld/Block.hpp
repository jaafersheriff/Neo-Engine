#pragma once
#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_

#include "Entity/Entity.hpp"
#include "Collision/BoundingSphere.hpp"

class Block : public Entity {
    public:
        Block(Mesh *, ModelTexture, const glm::vec3, const float, const glm::vec3, float);
        
        BoundingSphere boundingSphere;
        bool isHit;
        float velocity;
        glm::vec3 direction;
        
        /* Define model textures for dead and alive game objects*/

        void update(const float, const float, const float, const BoundingSphere &, std::vector<BoundingSphere *>);
};

#endif