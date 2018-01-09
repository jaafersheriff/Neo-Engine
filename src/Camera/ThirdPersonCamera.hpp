#pragma ocne
#ifndef _THIRD_PERSON_CAMERA_HPP_
#define _THIRD_PERSON_CAMERA_HPP_

#include "Camera/Camera.hpp"

#define MIN_DISTANCE 5
#define MAX_DISTANCE 100

class ThirdPersonCamera : public Camera {
    public:
        ThirdPersonCamera(glm::vec3 *);
        ThirdPersonCamera(glm::vec3 *, const glm::vec3);

        /* Update */
        virtual void update();
        virtual void updateLookAt();
        virtual void takeMouseInput(const float, const float);

        /* Move according to UVW */
        virtual void moveForward();
        virtual void moveBackward();
        virtual void moveLeft();
        virtual void moveRight();
        virtual void moveUp();
        virtual void moveDown();

      private:
         const glm::vec3 *lookRef;
         float distanceFromBase;
         float angleAroundBase;
};

#endif
