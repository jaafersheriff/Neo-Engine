#include "Player.hpp"

#include "glm/gtc/matrix_transform.hpp"

Player::Player(Camera *cam, BoundingSphere boundingBox) {
    this->camera = cam;
    this->boundingSphere = boundingBox;
}

void Player::update() {
    glm::mat4 M = glm::translate(glm::mat4(1.f), camera->position);
    this->boundingSphere.update(camera->position);
}