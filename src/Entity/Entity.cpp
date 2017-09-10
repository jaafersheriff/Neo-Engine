#include "Entity.hpp"

Entity::Entity(const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) {
   this->position = p;
   this->rotation = r;
   this->scale    = s;
}

void Entity::update() {

}