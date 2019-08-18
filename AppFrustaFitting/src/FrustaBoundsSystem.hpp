#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "FrustumBoundsComponent.hpp"

using namespace neo;

class FrustaBoundsSystem : public System {

public:
    FrustaBoundsSystem() :
        System("FrustaBounds System")
    {}

    virtual void update(const float dt) override {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto bounds = camera->getGameObject().getComponentByType<FrustumBoundsComponent>()) {
                auto spatial = camera->getGameObject().getSpatial();
                float nDis = camera->getNearFar().x;
                float fDis = camera->getNearFar().y;
                glm::vec3 P = spatial->getPosition();
                glm::vec3 v = glm::normalize(spatial->getLookDir());
                glm::vec3 up = glm::normalize(spatial->getUpDir());
                glm::vec3 w = glm::normalize(spatial->getRightDir());
                glm::vec3 Cnear = P + v * nDis;
                glm::vec3 Cfar = P + v * fDis;

                if (auto orthoCam = dynamic_cast<OrthoCameraComponent*>(camera)) {
                    bounds->NearLeftTop = Cnear + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().x);
                    bounds->NearRightTop = Cnear + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().y);
                    bounds->NearLeftBottom = Cnear + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().x);
                    bounds->NearRightBottom = Cnear + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().y);
                    bounds->FarLeftTop = Cfar + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().x);
                    bounds->FarRightTop = Cfar + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().y);
                    bounds->FarLeftBottom = Cfar + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().x);
                    bounds->FarRightBottom = Cfar + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().y);
                }
                else if (auto perspectiveCam = dynamic_cast<PerspectiveCameraComponent*>(camera)) {
                    float fov = glm::radians(perspectiveCam->getFOV());
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
