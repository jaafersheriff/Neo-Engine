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

Entity::Entity(Mesh *m, ModelTexture t, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) :
   Entity(m, p, r, s) {
   this->texture = t;
}

void Entity::update() {
}