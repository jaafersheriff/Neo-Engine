#include "Skybox.hpp"

Skybox::Skybox() {
    /* Create mesh */
    mesh = new Mesh;
    mesh->vertBuf = this->verts;
    mesh->init();
}

void Skybox::update(const float frameTime) {
    rotation += ROTATE_SPEED * frameTime;
}