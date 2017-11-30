#include "Billboard.cpp"

#define SQUARE_SIZE 0.5f
Billboard::Billboard(glm::vec3 c, glm::vec2 s) {
    this->center = c;
    this->size = s;

    /* Initialize square mesh */
    mesh = new Mesh;
    mesh->vertBuf = {
        -SQUARE_SIZE, -SQUARE_SIZE, 0.0f,
         SQUARE_SIZE, -SQUARE_SIZE, 0.0f,
        -SQUARE_SIZE,  SQUARE_SIZE, 0.0f,
         SQUARE_SIZE,  SQUARE_SIZE, 0.0f,
    };
    mesh->init();
}

Billboard::Billboard(Texture t, glm::vec3 c, glm::vec2 s) :
    Billboard(c, s) {
    this->texture = t;
}