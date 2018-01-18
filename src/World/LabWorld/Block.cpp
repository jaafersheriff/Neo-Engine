#include "Block.hpp"

#include "glm/gtc/matrix_transform.hpp"

Block::Block(Mesh *m, ModelTexture mt, const glm::vec3 p, const float r, const glm::vec3 s, float vel) : 
    Entity(m, mt, p, glm::vec3(0.f, r, 0.f), s) {
    this->isHit = false;
    this->direction = glm::vec3(
                            glm::rotate(
                                glm::mat4(1.f), 
                                glm::radians(r), 
                                glm::vec3(0, 1, 0)) * 
                            glm::vec4(0.f, 0.f, 1.f, 0.f));
    this->velocity = vel;
    this->boundingSphere = BoundingSphere(m);
}

void Block::update(Entity *terrain, BoundingSphere player, std::vector<Block *> blocks) {
    Entity::update();
    this->boundingSphere.update(this->position);

    /* If already hit, skip */
    if (isHit) {
        return;
    }

    /* Test for player collsiion */
    if (this->boundingSphere.intersect(player)) {
        isHit = true;
        this->modelTexture.diffuseColor = glm::vec3(1.f, 0.f, 0.f);
        return;
    }

    glm::vec3 newPos = this->position + this->direction * velocity;
    this->position = newPos;

    bool collision = newPos.x > terrain->scale.x || newPos.x < -terrain->scale.z ||
                     newPos.z > terrain->scale.x || newPos.z < -terrain->scale.z;
    for(auto block : blocks) {
        if (block != this && block->boundingSphere.intersect(this->boundingSphere)) {
            collision = true;
            break;
        }
    }

    if (collision) {
        this->velocity = -this->velocity;
        this->rotation.y += 180.f;
    }

    // TODO : collision w other blocks
    // TODO : collision w terrain
}