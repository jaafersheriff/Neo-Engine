#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include <algorithm>
#include <limits>

using namespace neo;

class FrustaFittingSystem : public System {

public:
    FrustaFittingSystem() :
        System("FrustaFitting System")
    {}

    bool updatePerspective = true;
    bool updateOrtho = true;

    struct BoundingBox {
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(-std::numeric_limits<float>::max());

        BoundingBox() {}

        BoundingBox(const FrustumBoundsComponent* bounds) {
            addNewPosition(bounds->NearLeftBottom);
            addNewPosition(bounds->NearLeftTop);
            addNewPosition(bounds->NearRightBottom);
            addNewPosition(bounds->NearRightTop);
            addNewPosition(bounds->FarLeftBottom);
            addNewPosition(bounds->FarLeftTop);
            addNewPosition(bounds->FarRightBottom);
            addNewPosition(bounds->FarRightTop);

        }

        void addNewPosition(glm::vec3 other) {
            if (other.x < min.x) { min.x = other.x; }
            if (other.y < min.y) { min.y = other.y; }
            if (other.z < min.z) { min.z = other.z; }
            if (other.x > max.x) { max.x = other.x; }
            if (other.y > max.y) { max.y = other.y; }
            if (other.z > max.z) { max.z = other.z; }
        }

        glm::vec3 center() {
            return glm::mix(min, max, 0.5f);
        }

        float width() {
            return max.x - min.x;
        }

        float height() {
            return max.y - min.y;
        }

        float depth() {
            return max.z - min.z;
        }
    };

    virtual void update(const float dt) override {
        auto mockOrthoCamera = Engine::getSingleComponent<MockOrthoComponent>();
        auto mockPerspectiveCamera = Engine::getSingleComponent<MockPerspectiveComponent>();
        if (!mockOrthoCamera || !mockPerspectiveCamera) {
            return;
        }
        auto orthoCamera = mockOrthoCamera->getGameObject().getComponentByType<CameraComponent>();
        auto perspectiveCamera = mockPerspectiveCamera->getGameObject().getComponentByType<CameraComponent>();
        if (!orthoCamera || !perspectiveCamera) {
            return;
        }
        auto orthoSpat = mockOrthoCamera->getGameObject().getSpatial();
        auto perspectiveSpat = mockPerspectiveCamera->getGameObject().getSpatial();
        if (!orthoSpat || !perspectiveSpat) {
            return;
        }

        if (updatePerspective) {
            float f = glm::sin(Util::getRunTime());
            float g = glm::cos(Util::getRunTime());
            perspectiveCamera->setLookDir(glm::vec3(f, f / 2, g));
        }

        /////////////////////// Do the fitting! ///////////////////////////////
        const glm::vec3 lightDir = glm::normalize(-orthoCamera->getLookDir());
        const glm::vec3 up = orthoCamera->getUpDir();

        const auto& sceneView = perspectiveCamera->getView();
        const auto& sceneProj = perspectiveCamera->getProj();
        const auto& worldToLight = glm::lookAt(orthoCamera->getLookDir(), glm::vec3(0.f), up);
        const auto& lightToWorld = glm::inverse(worldToLight);

        const float aspect = sceneProj[1][1] / sceneProj[0][0];
        const float fov = 2.f * glm::atan(1.f / sceneProj[1][1]);
        const float zNear = sceneProj[3][2] / (sceneProj[2][2] - 1.f);
        const float zFar = sceneProj[3][2] / (sceneProj[2][2] + 1.f);
        const float zRange = zFar - zNear; // std::min(mockOrthoCamera->range, zFar - zNear);
        const float depthMin = -1.f; // GL things?

        const std::vector<glm::vec4> corners = { // screen space ortho box 
            { -1.f,  1.f, depthMin, 1.f }, // corners of near plane
            {  1.f,  1.f, depthMin, 1.f },
            { -1.f, -1.f, depthMin, 1.f },
            {  1.f, -1.f, depthMin, 1.f },
            { -1.f,  1.f,      1.f, 1.f }, // corners of far plane
            {  1.f,  1.f,      1.f, 1.f },
            { -1.f, -1.f,      1.f, 1.f },
            {  1.f, -1.f,      1.f, 1.f }
        };

        glm::mat4 shadowToWorld = glm::inverse(sceneProj * sceneView); // scene view pos
        BoundingBox orthoBox;
        for (const auto& corner : corners) {
            glm::vec4 worldPos = shadowToWorld * corner; // transform corners of screen space unit ortho box into scene PV space
            worldPos = worldPos / worldPos.w;
            worldPos = worldToLight * worldPos; // rotate corners with light's world rotation
            orthoBox.addNewPosition(worldPos);
        }

        glm::vec3 center = lightToWorld * glm::vec4(orthoBox.center(), 1.f); // orthos center back in light space
        const float boxWidth = orthoBox.width() * 0.5f;
        const float boxHeight = orthoBox.height() * 0.5f;
        const float boxDepth = orthoBox.depth() * 0.5f;

        glm::vec3 shadowViewPos = glm::vec3(center) + (lightDir * mockOrthoCamera->distance);
        orthoSpat->setPosition(shadowViewPos);
        orthoCamera->setOrthoBounds(glm::vec2(-boxWidth, boxWidth), glm::vec2(-boxHeight, boxHeight));

        glm::vec3 midSceneView = perspectiveSpat->getPosition() + perspectiveCamera->getLookDir() * perspectiveCamera->getNearFar().y / 2.f;
        float shadowToSceneDistance = glm::distance(shadowViewPos, midSceneView);
        orthoCamera->setNearFar(-boxDepth + shadowToSceneDistance, boxDepth + shadowToSceneDistance);
    }
};
