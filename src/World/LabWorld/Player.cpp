#include "Player.hpp"

Player::Player(Camera *cam, AABB boundingBox) {
    this->camera = cam;
    this->boundingBox = boundingBox;
}

void Player::update() {
    this->boundingBox.position = camera->position;
}