// Camera controller that makes a camera always face 
// towards a provided spatial position. Reduces the 
// need for cross-component messaging

#pragma once

#include "Component/Component.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

class LookAtCameraController : public Component {

    public:
        LookAtCameraController(GameObject *go, CameraComponent *camera, SpatialComponent *lookAt) :
            Component(go),
            camera(camera),
            lookAt(lookAt) 
        {}

        void update(float) override {
            // TODO : this is being called on every frame
            // TODO : it should only be called on camera->SpatialChangeMessage or lookAt->SpatialChangeMessage
            glm::vec3 lookPos = lookAt->getPosition();
            if (auto spatial = this->getGameObject().getSpatial()) {
                glm::vec3 thisPos = spatial->getPosition();
                camera->setLookDir(lookPos - thisPos);
            }
        }

        void replaceLookAt(SpatialComponent *spat) { this->lookAt = spat; }

    private:
        CameraComponent * camera;
        SpatialComponent *lookAt;
};