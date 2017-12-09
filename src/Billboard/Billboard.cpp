#include "Billboard.hpp"

Billboard::Billboard(const glm::vec3 c, const glm::vec2 s) {
    this->center = c;
    this->size = s;

    /* Initialize square mesh */
    mesh = new Mesh;
    mesh->vertBuf = {
        -1.f, -1.f, 0.0f,
         1.f, -1.f, 0.0f,
        -1.f,  1.f, 0.0f,
         1.f,  1.f, 0.0f,
    };
    mesh->init();
}

Billboard::Billboard(Texture *t, const glm::vec3 c, const glm::vec2 s) :
    Billboard(c, s) {
    this->texture = t;
}
