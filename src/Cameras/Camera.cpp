#include "Camera.hpp"

Camera::Camera(const glm::vec3 position) {
   this->position = position;
   updateLookAt(0.f, 0.f);
   update();
}

// dx and dy correspond to mouse movement 
void Camera::updateLookAt(const float dx, const float dy) {
   // Update look at point
   theta += dx * lookSpeed;
   phi -= dy * lookSpeed;
   glm::vec3 sphere = glm::vec3(
                  glm::cos(phi)*glm::cos(theta), 
                  glm::sin(phi), 
                  glm::cos(phi)*glm::cos((Toolbox::PI/2.f)-theta));
   lookAt = position + glm::normalize(sphere);
}

// Update UVW
void Camera::update() {
   w = glm::normalize(lookAt - position) * moveSpeed;
   u = glm::normalize(glm::cross(w, glm::vec3(0, 1, 0))) * moveSpeed;
   v = glm::normalize(glm::cross(u, w)) * moveSpeed;
}

/* 
 * All movement is based on UVW basis-vectors
 */
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