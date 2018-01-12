#include "Entity.hpp"

Entity::Entity(const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) {
    this->position = p;
    this->rotation = r;
    this->scale    = s;
}

Entity::Entity(Mesh *m, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) : 
    Entity(p, r, s) {
    this->mesh = m;
}

Entity::Entity(Mesh *m, ModelTexture mt, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) :
    Entity(m, p, r, s) {
    this->modelTexture = mt;
}

Entity::Entity(Mesh *m, Texture *t, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) : 
    Entity(m, p, r,s ) {
    this->modelTexture.texture = t;
}

void Entity::update() {
    M = glm::translate(glm::mat4(1.f), e->position);
    M *= glm::rotate(glm::mat4(1.f), glm::radians(e->rotation.x), glm::vec3(1, 0, 0));
    M *= glm::rotate(glm::mat4(1.f), glm::radians(e->rotation.y), glm::vec3(0, 1, 0));
    M *= glm::rotate(glm::mat4(1.f), glm::radians(e->rotation.z), glm::vec3(0, 0, 1));
    M *= glm::scale(glm::mat4(1.f), e->scale);
}
