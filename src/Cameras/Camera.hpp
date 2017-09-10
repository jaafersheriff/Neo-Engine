// Parent Camera class
// This Camera can be used as a floating camera with no bounds
// Subclasses of this Camera can impose limiations/locks on this Camera  
#pragma once
#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include "glm/glm.hpp"
#include "Toolbox/Toolbox.hpp"

class Camera {
   public:
      float moveSpeed = 1.f;
      float phi, theta;
      glm::vec3 position;
      glm::vec3 lookAt;

      Camera();
      Camera(glm::vec3);

      void update(const float, const float);
      void moveForward();
      void moveBackward();
      void moveLeft();
      void moveRight();
      void moveUp();
      void moveDown();

   private:
      glm::vec3 u, v, w;
};

#endif