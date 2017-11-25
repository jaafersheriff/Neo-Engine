#include "Skybox.hpp"

Skybox::Skybox(CubeTexture cb) {
    /* Create mesh */
    mesh = new Mesh;
    mesh->vertBuf = this->verts;
    mesh->init();

    /* Set cube texture */
    this->cubeTexture = cb;
}

void Skybox::update(const float frameTime) {
    rotation += ROTATE_SPEED * frameTime;
}