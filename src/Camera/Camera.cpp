#include "Camera.hpp"

Camera::Camera(const glm::vec3 position) {
    /* Set position */
    this->position = position;
    /* Set UVW vector and lookAt */
    updateUVW();
    updateLookAt();
}

/* Update look at point
 * dx and dy correspond to mouse movement */
void Camera::takeMouseInput(const float dx, const float dy) {
    theta += dx * LOOK_SPEED;
    phi -= dy * LOOK_SPEED;
}

void Camera::update() {
    updateUVW();
    updateLookAt();
}

void Camera::updateUVW() {
    w = glm::normalize(lookAt - position);
    u = glm::normalize(glm::cross(w, glm::vec3(0, 1, 0)));
    v = glm::normalize(glm::cross(u, w));
}

void Camera::updateLookAt() {
    glm::vec3 sphere(
            glm::cos(phi)*glm::cos(theta),
            glm::sin(phi),
            glm::cos(phi)*glm::cos((Toolbox::PI/2.f)-theta));
    lookAt = position + glm::normalize(sphere);
}

/* All movement is based on UVW basis-vectors */
void Camera::moveForward() { 
    position += w * MOVE_SPEED;
    lookAt += w * MOVE_SPEED;
}

void Camera::moveBackward() { 
    position -= w * MOVE_SPEED;
    lookAt -= w * MOVE_SPEED;
}

void Camera::moveRight() { 
    position += u * MOVE_SPEED;
    lookAt += u * MOVE_SPEED;
}

void Camera::moveLeft() { 
    position -= u * MOVE_SPEED;
    lookAt -= u * MOVE_SPEED;
}

void Camera::moveUp() { 
    position += v * MOVE_SPEED;
    lookAt += v * MOVE_SPEED;
}

void Camera::moveDown() { 
    position -= v * MOVE_SPEED;
    lookAt -= v * MOVE_SPEED;
}
