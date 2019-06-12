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

    struct Planes {
        glm::vec4 Left, Right, Bottom, Top, Near, Far;
    };

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

    Planes ExtractVFPlanes(glm::mat4 P, glm::mat4 V) {
        Planes planes;
        glm::vec3 n; //use to pull out normal
        float l; //length of normal for plane normalization

        glm::mat4 comp = P * V;

        planes.Left.x = comp[0][3] + comp[0][0];
        planes.Left.y = comp[1][3] + comp[1][0];
        planes.Left.z = comp[2][3] + comp[2][0];
        planes.Left.w = comp[3][3] + comp[3][0];
        n = glm::vec3(planes.Left.x, planes.Left.y, planes.Left.z);
        l = length(n);
        planes.Left /= l;

        planes.Right.x = comp[0][3] - comp[0][3];
        planes.Right.y = comp[1][3] - comp[1][3];
        planes.Right.z = comp[2][3] - comp[2][3];
        planes.Right.w = comp[3][3] - comp[3][3];
        n = glm::vec3(planes.Right.x, planes.Right.y, planes.Right.z);
        l = length(n);
        planes.Right /= l;

        planes.Bottom.x = comp[0][3] + comp[0][1];
        planes.Bottom.y = comp[1][3] + comp[1][1];
        planes.Bottom.z = comp[2][3] + comp[2][1];
        planes.Bottom.w = comp[3][3] + comp[3][1];
        n = glm::vec3(planes.Bottom.x, planes.Bottom.y, planes.Bottom.z);
        l = length(n);
        planes.Bottom /= l;

        planes.Top.x = comp[0][3] - comp[0][1];
        planes.Top.y = comp[1][3] - comp[1][1];
        planes.Top.z = comp[2][3] - comp[2][1];
        planes.Top.w = comp[3][3] - comp[3][1];
        n = glm::vec3(planes.Top.x, planes.Top.y, planes.Top.z);
        l = length(n);
        planes.Top /= l;

        planes.Near.x = comp[0][2];
        planes.Near.y = comp[1][2];
        planes.Near.z = comp[2][2];
        planes.Near.w = comp[3][2];
        n = glm::vec3(planes.Near.x, planes.Near.y, planes.Near.z);
        l = length(n);
        planes.Near /= l;

        planes.Far.x = comp[0][3] - comp[0][2];
        planes.Far.y = comp[1][3] - comp[1][2];
        planes.Far.z = comp[2][3] - comp[2][2];
        planes.Far.w = comp[3][3] - comp[3][2];
        n = glm::vec3(planes.Far.x, planes.Far.y, planes.Far.z);
        l = length(n);
        planes.Far /= l;

        return planes;
    }

    glm::vec3 IntersectPlanes(glm::vec4 P0, glm::vec4 P1, glm::vec4 P2) {
        glm::vec3 bxc = glm::cross(glm::vec3(P1), glm::vec3(P2));
        glm::vec3 cxa = glm::cross(glm::vec3(P2), glm::vec3(P0));
        glm::vec3 axb = glm::cross(glm::vec3(P0), glm::vec3(P1));
        glm::vec3 r = -P0.w * bxc - P1.w * cxa - P2.w * axb;
        return r * (1 / glm::dot(glm::vec3(P0), bxc));
    }

    IntersectionPoints getPoints(Planes planes) {
        IntersectionPoints points;
        points.NearLeftBottom = IntersectPlanes(planes.Near, planes.Left, planes.Bottom);
        points.NearLeftTop = IntersectPlanes(planes.Near, planes.Left, planes.Top);
        points.NearRightBottom = IntersectPlanes(planes.Near, planes.Right, planes.Bottom);
        points.NearRightTop = IntersectPlanes(planes.Near, planes.Right, planes.Top);
        points.FarLeftBottom = IntersectPlanes(planes.Far, planes.Left, planes.Bottom);
        points.FarLeftTop = IntersectPlanes(planes.Far, planes.Left, planes.Top);
        points.FarRightTop = IntersectPlanes(planes.Far, planes.Right, planes.Top);
        points.FarRightBottom = IntersectPlanes(planes.Far, planes.Right, planes.Bottom);
        return points;
    }


    virtual void update(const float dt) override {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto line = camera->getGameObject().getComponentByType<LineComponent>()) {
                line->clearNodes();

                IntersectionPoints points = getPoints(ExtractVFPlanes(camera->getProj(), camera->getView()));
                // Near
                line->addNode(points.NearLeftBottom);  line->addNode(points.NearLeftTop);
                line->addNode(points.NearLeftTop);     line->addNode(points.NearRightTop);
                line->addNode(points.NearRightTop);    line->addNode(points.NearRightBottom);
                line->addNode(points.NearRightBottom); line->addNode(points.NearLeftBottom);

                // Archs
                line->addNode(points.NearLeftBottom);  line->addNode(points.FarLeftBottom);
                line->addNode(points.NearLeftTop);     line->addNode(points.FarLeftTop);
                line->addNode(points.NearRightBottom); line->addNode(points.FarRightBottom);
                line->addNode(points.NearRightTop);    line->addNode(points.FarRightTop);

                // Far
                line->addNode(points.FarLeftBottom);  line->addNode(points.FarLeftTop);
                line->addNode(points.FarLeftTop);     line->addNode(points.FarRightTop);
                line->addNode(points.FarRightTop);    line->addNode(points.FarRightBottom);
                line->addNode(points.FarRightBottom); line->addNode(points.FarLeftBottom);
            }
        }

    }

};
