#include "ECS/pch.hpp"
#include "Engine/Engine.hpp"
#include "FrustumSystem.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {
    void FrustumSystem::update(ECS& ecs) {
        for (auto&& [entity, frustum, spatial] : ecs.getView<FrustumComponent, SpatialComponent>().each()) {

            glm::vec3 P = spatial.getPosition();
            glm::vec3 v = glm::normalize(spatial.getLookDir());
            glm::vec3 up = glm::normalize(spatial.getUpDir());
            glm::vec3 w = glm::normalize(spatial.getRightDir());

            // Update frustum bounds for camera type
            glm::mat4 PV(1.f);
            if (auto orthoCam = ecs.getComponent<OrthoCameraComponent>(entity)) {
                PV = orthoCam->getProj() * spatial.getView();
                float nDis = orthoCam->getNearFar().x;
                float fDis = orthoCam->getNearFar().y;
                glm::vec3 Cnear = P + v * nDis;
                glm::vec3 Cfar = P + v * fDis;

                frustum.NearLeftTop = Cnear + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().x);
                frustum.NearRightTop = Cnear + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().y);
                frustum.NearLeftBottom = Cnear + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().x);
                frustum.NearRightBottom = Cnear + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().y);
                frustum.FarLeftTop = Cfar + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().x);
                frustum.FarRightTop = Cfar + (up * orthoCam->getVerticalBounds().y) + (w * orthoCam->getHorizontalBounds().y);
                frustum.FarLeftBottom = Cfar + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().x);
                frustum.FarRightBottom = Cfar + (up * orthoCam->getVerticalBounds().x) + (w * orthoCam->getHorizontalBounds().y);

                frustum.mNear.x = PV[0][3] + PV[0][2];
                frustum.mNear.y = PV[1][3] + PV[1][2];
                frustum.mNear.z = PV[2][3] + PV[2][2];
                frustum.mNear.w = PV[3][3] + PV[3][2];
                // frustum.mNear /= glm::length(glm::vec3(frustum.mNear));
            }
            else if (auto perspectiveCam = ecs.getComponent<PerspectiveCameraComponent>(entity)) {
                PV = perspectiveCam->getProj() * spatial.getView();
                float nDis = perspectiveCam->getNearFar().x;
                float fDis = perspectiveCam->getNearFar().y;
                glm::vec3 Cnear = P + v * nDis;
                glm::vec3 Cfar = P + v * fDis;

                float fov = glm::radians(perspectiveCam->getFOV());
                float ar = perspectiveCam->getAspectRatio();
                float Hnear = 2 * glm::tan(fov / 2) * nDis;
                float Wnear = Hnear * ar;
                float Hfar = 2 * glm::tan(fov / 2) * fDis;
                float Wfar = Hfar * ar;
                frustum.NearLeftTop = Cnear + (up * (Hnear / 2)) - (w * (Wnear / 2));
                frustum.NearRightTop = Cnear + (up * (Hnear / 2)) + (w * (Wnear / 2));
                frustum.NearLeftBottom = Cnear - (up * (Hnear / 2)) - (w * (Wnear / 2));
                frustum.NearRightBottom = Cnear - (up * (Hnear / 2)) + (w * (Wnear / 2));
                frustum.FarLeftTop = Cfar + (up * (Hfar / 2)) - (w * (Wfar / 2));
                frustum.FarRightTop = Cfar + (up * (Hfar / 2)) + (w * (Wfar / 2));
                frustum.FarLeftBottom = Cfar - (up * (Hfar / 2)) - (w * (Wfar / 2));
                frustum.FarRightBottom = Cfar - (up * (Hfar / 2)) + (w * (Wfar / 2));

                frustum.mNear.x = PV[0][2];
                frustum.mNear.y = PV[1][2];
                frustum.mNear.z = PV[2][2];
                frustum.mNear.w = PV[3][2];
                // frustum.mNear /= glm::length(glm::vec3(frustum.mNear));
            }

            // Update frustum planes
            frustum.mLeft.x = PV[0][3] + PV[0][0];
            frustum.mLeft.y = PV[1][3] + PV[1][0];
            frustum.mLeft.z = PV[2][3] + PV[2][0];
            frustum.mLeft.w = PV[3][3] + PV[3][0];
            // frustum.mLeft /= glm::length(glm::vec3(frustum.mLeft));

            frustum.mRight.x = PV[0][3] - PV[0][0];
            frustum.mRight.y = PV[1][3] - PV[1][0];
            frustum.mRight.z = PV[2][3] - PV[2][0];
            frustum.mRight.w = PV[3][3] - PV[3][0];
            // frustum.mRight /= glm::length(glm::vec3(frustum.mRight));

            frustum.mBottom.x = PV[0][3] + PV[0][1];
            frustum.mBottom.y = PV[1][3] + PV[1][1];
            frustum.mBottom.z = PV[2][3] + PV[2][1];
            frustum.mBottom.w = PV[3][3] + PV[3][1];
            // frustum.mBottom /= glm::length(glm::vec3(frustum.mBottom));

            frustum.mTop.x = PV[0][3] - PV[0][1];
            frustum.mTop.y = PV[1][3] - PV[1][1];
            frustum.mTop.z = PV[2][3] - PV[2][1];
            frustum.mTop.w = PV[3][3] - PV[3][1];
            // frustum.mTop /= glm::length(glm::vec3(frustum.mTop));

            frustum.mFar.x = PV[0][3] - PV[0][2];
            frustum.mFar.y = PV[1][3] - PV[1][2];
            frustum.mFar.z = PV[2][3] - PV[2][2];
            frustum.mFar.w = PV[3][3] - PV[3][2];
            // frustum.mFar /= glm::length(glm::vec3(frustum.mFar));
        }
    }
}
