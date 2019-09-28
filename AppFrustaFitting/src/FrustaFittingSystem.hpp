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

    struct BoundingBox {
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(-std::numeric_limits<float>::max());
        std::vector<glm::vec3> points;

        BoundingBox() {}

        BoundingBox(const FrustumComponent* bounds) {
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
            points.push_back(other);
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

    bool updatePerspective = true;
    bool updateOrtho = true;

    FrustaFittingSystem(float texelSize) :
        System("FrustaFitting System") {
    }

    virtual void update(const float dt) override {
        const auto shadowCamera = Engine::getSingleComponent<ShadowCameraComponent>();
        const auto mainCamera = Engine::getSingleComponent<MainCameraComponent>();
        auto light = Engine::getSingleComponent<LightComponent>();
        if (!shadowCamera || !mainCamera || !light) {
            return;
        }
        auto orthoCamera = dynamic_cast<OrthoCameraComponent*>(shadowCamera->getGameObject().getComponentByType<CameraComponent>());
        auto perspectiveCamera = dynamic_cast<PerspectiveCameraComponent*>(mainCamera->getGameObject().getComponentByType<CameraComponent>());
        if (!orthoCamera || !perspectiveCamera) {
            return;
        }
        auto orthoSpat = orthoCamera->getGameObject().getComponentByType<SpatialComponent>();
        auto perspectiveSpat = perspectiveCamera->getGameObject().getComponentByType<SpatialComponent>();
        if (!orthoSpat || !perspectiveSpat) {
            return;
        }

        if (updatePerspective) {
            float f = glm::sin(Util::getRunTime());
            float g = glm::cos(Util::getRunTime());
            perspectiveSpat->setLookDir(glm::vec3(f, f / 2, g));
        }

        /////////////////////// Do the fitting! ///////////////////////////////
        const auto lightSpat = light->getGameObject().getComponentByType<SpatialComponent>();
        if (!lightSpat) {
            return;
        }
        const glm::vec3 lightDir = lightSpat->getLookDir();
        const glm::vec3 up = lightSpat->getUpDir();

        const auto& sceneView = perspectiveCamera->getView();
        const auto& sceneProj = perspectiveCamera->getProj();
        const auto& worldToLight = glm::lookAt(lightDir, glm::vec3(0.f), up);
        const auto& lightToWorld = glm::inverse(worldToLight);

        const float aspect = sceneProj[1][1] / sceneProj[0][0];
        const float fov = 2.f * glm::atan(1.f / sceneProj[1][1]);
        const float zNear = sceneProj[3][2] / (sceneProj[2][2] - 1.f);
        const float zFar = sceneProj[3][2] / (sceneProj[2][2] + 1.f);
        const float zRange = zFar - zNear;
        const float depthMin = -1.f; // GL things

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


        // orthoSpat->setPosition(center);
        // orthoCamera->setOrthoBounds(glm::vec2(-boxWidth, boxWidth), glm::vec2(-boxHeight, boxHeight));
        // orthoCamera->setNearFar(-boxDepth, boxDepth);

        glm::vec3 midSceneView = perspectiveSpat->getPosition() + perspectiveSpat->getLookDir() * perspectiveCamera->getNearFar().y / 2.f;
        float shadowToSceneDistance = glm::distance(lightSpat->getPosition(), midSceneView);

        orthoSpat->setPosition(center);
        orthoSpat->setLookDir(lightDir);
        orthoCamera->setOrthoBounds(glm::vec2(-boxWidth, boxWidth), glm::vec2(-boxHeight, boxHeight));
        orthoCamera->setNearFar(-boxDepth, boxDepth);

    }
};
