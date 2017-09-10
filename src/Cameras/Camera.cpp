#include "Camera.hpp"

Camera::Camera(glm::vec3 position) {
   this->position = position;
   update(0.f, 0.f);
}

Camera::Camera() {
   Camera(glm::vec3(0, 0, 0));
}

// dx and dy correspond to mouse movement 
void Camera::update(const float dx, const float dy) {
   // Update look at point
   theta += dx * moveSpeed;
   phi += dy * moveSpeed;
   glm::vec3 sphere = glm::vec3(
                  glm::cos(phi)*glm::cos(theta), 
                  glm::sin(phi), 
                  glm::cos(phi)*glm::cos(PI/2.f)-theta);
   lookAt = position + glm::normalize(sphere);

   // Update UVW
   w = glm::normalize(lookAt - position);
   u = glm::normalize(glm::cross(w, glm::vec3(0, 1, 0)));
   v = glm::normalize(glm::cross(u, w));
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