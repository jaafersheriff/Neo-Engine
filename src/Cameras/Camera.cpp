#include "Camera.hpp"

Camera::Camera(const glm::vec3 position) {
    /* Set position */
    this->position = position;
    /* Set look at point */
    updateLookAt(0.f, 0.f);
    /* Set UVW vector */
    update();
}

/* Update look at point
 * dx and dy correspond to mouse movement */
void Camera::updateLookAt(const float dx, const float dy) {
    theta += dx * LOOK_SPEED;
    phi -= dy * LOOK_SPEED;
    glm::vec3 sphere(
            glm::cos(phi)*glm::cos(theta),
            glm::sin(phi),
            glm::cos(phi)*glm::cos((Toolbox::PI/2.f)-theta));
    lookAt = position + glm::normalize(sphere);
}

/* Update UVW */
void Camera::update() {
    w = glm::normalize(lookAt - position) * MOVE_SPEED;
    u = glm::normalize(glm::cross(w, glm::vec3(0, 1, 0))) * MOVE_SPEED;
    v = glm::normalize(glm::cross(u, w)) * MOVE_SPEED;
}

/* All movement is based on UVW basis-vectors */
void Camera::moveForward() { 
    position += w;
    lookAt += w;
}

void Camera::moveBackward() { 
    position -= w;
    lookAt -= w;
}

void Camera::moveRight() { 
    position += u;
    lookAt += u;
}

void Camera::moveLeft() { 
    position -= u;
    lookAt -= u;
}

void Camera::moveUp() { 
    position += v;
    lookAt += v;
}

void Camera::moveDown() { 
    position -= v;
    lookAt -= v;
}