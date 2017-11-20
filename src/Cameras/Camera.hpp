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
        float lookSpeed = 0.005f;
        float moveSpeed = 0.5f;
        glm::vec3 position;
        glm::vec3 lookAt;

        Camera(const glm::vec3);
        Camera() : Camera(glm::vec3(0)) { }

        // Update
        void update();
        void updateLookAt(const float, const float);

        // Move according to UVW
        void moveForward();
        void moveBackward();
        void moveLeft();
        void moveRight();
        void moveUp();
        void moveDown();

    protected:
        double phi = 0.0;
        double theta = 0.0;
        glm::vec3 u, v, w;
};

#endif