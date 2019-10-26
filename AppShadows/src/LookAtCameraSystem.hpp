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
            auto shadowCamera = Engine::getComponentTuple<ShadowCameraComponent, CameraComponent>();
            if (!cameraLookPos || !shadowCamera) {
                return;
            }

            auto camera = shadowCamera->get<CameraComponent>();
            glm::vec3 lookPos = cameraLookPos->getGameObject().getComponentByType<SpatialComponent>()->getPosition();
            cameraLookPos->getGameObject().getComponentByType<SpatialComponent>()->setLookDir(lookPos - camera->getGameObject().getComponentByType<SpatialComponent>()->getPosition());
        }

};
