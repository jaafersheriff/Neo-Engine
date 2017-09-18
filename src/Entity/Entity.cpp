#include "Entity.hpp"

#include <iostream> // TODO : delete

Entity::Entity(const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) {
   this->mesh = NULL;
   this->position = p;
   this->rotation = r;
   this->scale    = s;
}

Entity::Entity(Mesh *m, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) : 
   Entity(p, r, s) {
   this->mesh = m;
}

void Entity::update() {

}