#include "Block.hpp"
Block::Block(Mesh *m, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s, float vel) : 
    Entity(m, alive, p, r, s) {
    this->velocity = vel;
    this->isHit = false;
    this->boundingBox = AABB(m, p);
}

void Block::update() {
    if (isHit) {
        return;
    }
    // if (this->intersects(player)) { isHit = true; update modeltexture; return }
    //glm::vec3 newPos =  this->position = this->rotation * velocity;
    // if (newPos.x > terrain.scale || newPos.x < -terrain.scale || 
    //     newPos.z > terrain.scale || newPos.z < -terrain.scale) { 
    //  vel = -vel }
    //this->position = newPos;
}