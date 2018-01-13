#include "Block.hpp"
Block::Block(Mesh *m, ModelTexture mt, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s, float vel) : 
    Entity(m, mt, p, r, s) {
    this->velocity = vel;
    this->isHit = false;
    this->boundingBox = BoundingBox(m);
}

void Block::update(Entity *terrain, BoundingBox player) {
    Entity::update();
    this->boundingBox.update(&this->M);

    /* If already hit, skip */
    if (isHit) {
        return;
    }

    /* Test for player collsiion */
    if (this->boundingBox.intersect(player)) {
        isHit = true;
        this->modelTexture.diffuseColor = glm::vec3(1.f, 0.f, 0.f);
        return;
    }

    // TODO : update direction/position
    //glm::vec3 newPos =  this->position = this->rotation * velocity;
    // if (newPos.x > terrain.scale || newPos.x < -terrain.scale || 
    //     newPos.z > terrain.scale || newPos.z < -terrain.scale) { 
    //  vel = -vel }
    //this->position = newPos;

    // TODO : collision w other blocks
    // TODO : collision w terrain
}