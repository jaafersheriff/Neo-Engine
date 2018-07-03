#include "Skybox.hpp"

Skybox::Skybox(Mesh *m, CubeTexture *cb) {
    /* Create mesh */
    this->mesh = m;

    /* Set cube texture */
    cubeTexture = cb;
}

void Skybox::update(const float frameTime) {
    rotation += ROTATE_SPEED * frameTime;
}