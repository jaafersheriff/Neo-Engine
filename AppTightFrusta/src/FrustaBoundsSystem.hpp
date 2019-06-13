#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "FrustaBoundsComponent.hpp"

using namespace neo;

class FrustaBoundsSystem : public System {

public:
    FrustaBoundsSystem() :
        System("FrustaBounds System")
    {}

    virtual void update(const float dt) override {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto bounds = camera->getGameObject().getComponentByType<FrustaBoundsComponent>()) {
                float nDis = camera->getNearFar().x;
                float fDis = camera->getNearFar().y;
                float fov = camera->getFOV();
                glm::vec3 P = camera->getGameObject().getSpatial()->getPosition();
                glm::vec3 v = camera->getLookDir();
                glm::vec3 up = camera->getUpDir();
                glm::vec3 w = camera->getRightDir();
                glm::vec3 Cnear = P + v * nDis;
                glm::vec3 Cfar = P + v * fDis;

                if (camera->mIsOrtho) {
                    bounds->NearLeftTop = Cnear + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().x);
                    bounds->NearRightTop = Cnear + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().y);
                    bounds->NearLeftBottom = Cnear + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().x);
                    bounds->NearRightBottom = Cnear + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().y);
                    bounds->FarLeftTop = Cfar + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().x);
                    bounds->FarRightTop = Cfar + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().y);
                    bounds->FarLeftBottom = Cfar + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().x);
                    bounds->FarRightBottom = Cfar + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().y);
                }
                else {
                    float ar = 1.f;
                    float Hnear = 2 * glm::tan(fov / 2) * nDis;
                    float Wnear = Hnear * ar;
                    float Hfar = 2 * glm::tan(fov / 2) * fDis;
                    float Wfar = Hfar * ar;
                    bounds->NearLeftTop = Cnear + (up * (Hnear / 2)) - (w * (Wnear / 2));
                    bounds->NearRightTop = Cnear + (up * (Hnear / 2)) + (w * (Wnear / 2));
                    bounds->NearLeftBottom = Cnear - (up * (Hnear / 2)) - (w * (Wnear / 2));
                    bounds->NearRightBottom = Cnear - (up * (Hnear / 2)) + (w * (Wnear / 2));
                    bounds->FarLeftTop = Cfar + (up * (Hfar / 2)) - (w * (Wfar / 2));
                    bounds->FarRightTop = Cfar + (up * (Hfar / 2)) + (w * (Wfar / 2));
                    bounds->FarLeftBottom = Cfar - (up * (Hfar / 2)) - (w * (Wfar / 2));
                    bounds->FarRightBottom = Cfar - (up * (Hfar / 2)) + (w * (Wfar / 2));
                }
            }
        }
    }
};
