#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "LookAtCameraReceiver.hpp"
#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

class LookAtCameraSystem : public neo::System {

    public:
        LookAtCameraSystem() :
            neo::System("LookAtCamera System")
        {}

        virtual void update(const float dt) override {
            auto cameraLookPos = neo::Engine::getSingleComponent<LookAtCameraReceiver>();
            auto shadowCamera = neo::Engine::getSingleComponent<neo::ShadowCameraComponent>();
            if (!cameraLookPos || !shadowCamera) {
                return;
            }

            auto camera = shadowCamera->getGameObject().getComponentByType<neo::CameraComponent>();
            if (!camera) {
                return;
            }

            glm::vec3 lookPos = cameraLookPos->getGameObject().getSpatial()->getPosition();
            camera->setLookDir(lookPos - camera->getGameObject().getSpatial()->getPosition());
        }

};
