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

void Block::update(const float timeStep, const float tXBound, const float tZBound, const BoundingSphere &player, 
        std::vector<BoundingSphere *> spheres) {
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

    /* Calculate new position */
    glm::vec3 newPos = this->position + this->direction * velocity * timeStep;

    /* Test for terrain collision */
    bool collision = newPos.x > tXBound || newPos.x < -tXBound ||
                     newPos.z > tZBound || newPos.z < -tZBound;

    /* Test for block collision */
    for (auto sphere : spheres) {
        if (collision || (sphere != &this->boundingSphere && this->boundingSphere.intersect(*sphere))) {
            collision = true;
            break;
        }
    }

    if (collision) {
        this->velocity = -this->velocity;
        newPos = this->position + this->direction * velocity * timeStep;
        this->rotation.y += 180.f;
    }

    this->position = newPos;
}