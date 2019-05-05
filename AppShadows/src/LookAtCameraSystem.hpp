#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "LookAtCameraReceiver.hpp"
#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

class LookAtCameraSystem : public System {

    public:
        LookAtCameraSystem() :
            System("LookAtCamera System")
        {}

        virtual void update(const float dt) override {
            auto cameraLookPos = Engine::getSingleComponent<LookAtCameraReceiver>();
            auto shadowCamera = Engine::getSingleComponent<ShadowCameraComponent>();
            if (!cameraLookPos || !shadowCamera) {
                return;
            }

            auto camera = shadowCamera->getGameObject().getComponentByType<CameraComponent>();
            if (!camera) {
                return;
            }

            glm::vec3 lookPos = cameraLookPos->getGameObject().getSpatial()->getPosition();
            camera->setLookDir(lookPos - camera->getGameObject().getSpatial()->getPosition());
        }

};
