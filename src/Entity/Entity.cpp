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
    this->texture = mt;
}

Entity::Entity(Mesh *m, Texture t, const glm::vec3 p, const glm::vec3 r, const glm::vec3 s) : 
    Entity(m, p, r,s ) {
    this->texture.textureImage = t;
}

void Entity::update() {
    
}
