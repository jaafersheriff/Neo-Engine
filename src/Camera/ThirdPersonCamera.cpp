#include "ThirdPersonCamera.hpp"

ThirdPersonCamera::ThirdPersonCamera(glm::vec3 *ref) {
    this->lookRef = ref;
    update();
}

ThirdPersonCamera::ThirdPersonCamera(glm::vec3 *ref, const glm::vec3 pos) :
    ThirdPersonCamera(ref) {
    this->position = pos;
}

void ThirdPersonCamera::update() {
    Camera::updateUVW();
    updateLookAt();
    distanceFromBase = glm::distance(position, lookAt);
    
    float thetaR = glm::radians(theta);
    float phiR = glm::radians(phi);
    float horz = distanceFromBase * glm::cos(phiR);
    float vert = distanceFromBase * glm::sin(phiR);
    
    position.x = lookAt.x - horz * glm::sin(thetaR);
    position.z = lookAt.z - horz * glm::cos(thetaR);
    position.y = lookAt.y + vert;
}

void ThirdPersonCamera::takeMouseInput(const float dx, const float dy) {
    theta -= dx * MOVE_SPEED; // angle around base
    phi += dy * MOVE_SPEED;   // pitch
}

void ThirdPersonCamera::updateLookAt() {
    lookAt = *lookRef;
}

void ThirdPersonCamera::moveForward() {
    glm::vec3 newPos = position + w * MOVE_SPEED;
    if (glm::distance(newPos, lookAt) > MIN_DISTANCE) {
        position = newPos;
    }
}

void ThirdPersonCamera::moveBackward() {
    glm::vec3 newPos = position - w * MOVE_SPEED;
    if (glm::distance(newPos, lookAt) < MAX_DISTANCE) {
        position = newPos;
    }
}

void ThirdPersonCamera::moveLeft() {
    theta -= MOVE_SPEED;
}

void ThirdPersonCamera::moveRight() {
    theta += MOVE_SPEED;
}

void ThirdPersonCamera::moveUp() {
    phi += MOVE_SPEED;
}

void ThirdPersonCamera::moveDown() {
    phi -= MOVE_SPEED;
}

