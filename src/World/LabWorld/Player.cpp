#include "Player.hpp"

#include "glm/gtc/matrix_transform.hpp"

Player::Player(Camera *cam, BoundingBox boundingBox) {
    this->camera = cam;
    this->boundingBox = boundingBox;
}

void Player::update() {
    glm::mat4 M = glm::translate(glm::mat4(1.f), camera->position);
}