#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

class CameraLineSystem : public System {

public:
    CameraLineSystem() :
        System("CameraLine System")
    {}

    struct IntersectionPoints {
        glm::vec3 NearLeftBottom;
        glm::vec3 NearLeftTop;
        glm::vec3 NearRightBottom;
        glm::vec3 NearRightTop;
        glm::vec3 FarLeftBottom;
        glm::vec3 FarLeftTop;
        glm::vec3 FarRightBottom;
        glm::vec3 FarRightTop;
    };

    IntersectionPoints getPerspectivePoints(const CameraComponent *camera) {
        float nDis = camera->getNearFar().x;
        float fDis = camera->getNearFar().y;
        float fov = camera->getFOV();
        float ar = 1.f;
        glm::vec3 P = camera->getGameObject().getSpatial()->getPosition();
        glm::vec3 v = camera->getLookDir();
        glm::vec3 up = camera->getUpDir();
        glm::vec3 w = camera->getRightDir();

        float Hnear = 2 * glm::tan(fov / 2) * nDis;
        float Wnear = Hnear * ar;
        float Hfar = 2 * glm::tan(fov / 2) * fDis;
        float Wfar = Hfar * ar;
        glm::vec3 Cnear = P + v * nDis;
        glm::vec3 Cfar = P + v * fDis;

        IntersectionPoints points;
        points.NearLeftTop = Cnear + (up * (Hnear / 2)) - (w * (Wnear / 2));
        points.NearRightTop = Cnear + (up * (Hnear / 2)) + (w * (Wnear / 2));
        points.NearLeftBottom = Cnear - (up * (Hnear / 2)) - (w * (Wnear / 2));
        points.NearRightBottom = Cnear - (up * (Hnear / 2)) + (w * (Wnear / 2));
        points.FarLeftTop = Cfar + (up * (Hfar / 2)) - (w * (Wfar / 2));
        points.FarRightTop = Cfar + (up * (Hfar / 2)) + (w * (Wfar / 2));
        points.FarLeftBottom = Cfar - (up * (Hfar / 2)) - (w * (Wfar / 2));
        points.FarRightBottom = Cfar - (up * (Hfar / 2)) + (w * (Wfar / 2));
        return points;
    }

    IntersectionPoints getOrthoPoints(const CameraComponent *camera) {
        float nDis = camera->getNearFar().x;
        float fDis = camera->getNearFar().y;
        float ar = 1.f;
        glm::vec3 P = camera->getGameObject().getSpatial()->getPosition();
        glm::vec3 v = camera->getLookDir();
        glm::vec3 up = camera->getUpDir();
        glm::vec3 w = camera->getRightDir();

        glm::vec3 Cnear = P + v * nDis;
        glm::vec3 Cfar = P + v * fDis;

        IntersectionPoints points;
        points.NearLeftTop = Cnear + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().x);
        points.NearRightTop = Cnear + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().y);
        points.NearLeftBottom = Cnear + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().x);
        points.NearRightBottom = Cnear + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().y);
        points.FarLeftTop = Cfar + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().x);
        points.FarRightTop = Cfar + (up * camera->getHorizontalBounds().y) + (w * camera->getHorizontalBounds().y);
        points.FarLeftBottom = Cfar + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().x);
        points.FarRightBottom = Cfar + (up * camera->getHorizontalBounds().x) + (w * camera->getHorizontalBounds().y);
        return points;
    }

    virtual void update(const float dt) override {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto line = camera->getGameObject().getComponentByType<LineComponent>()) {

                IntersectionPoints points;
                if (camera->mIsOrtho) {
                    points = getOrthoPoints(camera);
                }
                else {
                    points = getPerspectivePoints(camera);
                }

                line->clearNodes();

                // We have to do this ridiculousness because line strip (:
                line->addNode(points.NearLeftBottom);
                line->addNode(points.NearLeftTop);
                line->addNode(points.NearRightTop);
                line->addNode(points.NearRightBottom);
                line->addNode(points.NearLeftBottom);
                line->addNode(points.FarLeftBottom);
                line->addNode(points.FarLeftTop);
                line->addNode(points.NearLeftTop);
                line->addNode(points.FarLeftTop);
                line->addNode(points.FarRightTop);
                line->addNode(points.NearRightTop);
                line->addNode(points.FarRightTop);
                line->addNode(points.FarRightBottom);
                line->addNode(points.NearRightBottom);
                line->addNode(points.FarRightBottom);
                line->addNode(points.FarLeftBottom);

            }
        }

    }

};
