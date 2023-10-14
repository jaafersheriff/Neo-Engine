#include "ECS/pch.hpp"
#include "FrustaFittingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    void FrustaFittingSystem::update(ECS& ecs) {
        ZoneScoped;

        auto sourceCameraTuple = ecs.getSingleView<FrustumFitSourceComponent, SpatialComponent, PerspectiveCameraComponent>();
        auto receiverCameraTuple = ecs.getSingleView<FrustumFitReceiverComponent, SpatialComponent, OrthoCameraComponent>();
        auto lightTuple = ecs.getSingleView<LightComponent, SpatialComponent>();
        if (!receiverCameraTuple || !sourceCameraTuple || !lightTuple) {
            return;
        }
        auto&& [sourceCameraEntity, _, sourceSpatial, sourceCamera] = *sourceCameraTuple;
        auto&& [receiverCameraEntity, receiverFrustum, receiverSpatial, receiverCamera] = *receiverCameraTuple;
        auto&& [lightEntity, light, lightSpatial] = *lightTuple;

        /////////////////////// Do the fitting! ///////////////////////////////
        const glm::vec3 lightDir = lightSpatial.getLookDir();
        const glm::vec3 up = lightSpatial.getUpDir();

        const auto& sourceView = sourceSpatial.getView();
        const auto& sourceProj = sourceCamera.getProj();
        const auto& worldToLight = glm::lookAt(lightDir, glm::vec3(0.f), up);
        const auto& lightToWorld = glm::inverse(worldToLight);

        // const float aspect = sourceProj[1][1] / sourceProj[0][0];
        // const float fov = 2.f * glm::atan(1.f / sourceProj[1][1]);
        // const float zNear = sourceProj[3][2] / (sourceProj[2][2] - 1.f);
        // const float zFar = sourceProj[3][2] / (sourceProj[2][2] + 1.f);
        // const float zRange = zFar - zNear;
        const float depthMin = -1.f; // GL things

        struct BoundingBox {
            glm::vec3 mMin = glm::vec3(std::numeric_limits<float>::max());
            glm::vec3 mMax = glm::vec3(-std::numeric_limits<float>::max());

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
                if (other.x < mMin.x) { mMin.x = other.x; }
                if (other.y < mMin.y) { mMin.y = other.y; }
                if (other.z < mMin.z) { mMin.z = other.z; }
                if (other.x > mMax.x) { mMax.x = other.x; }
                if (other.y > mMax.y) { mMax.y = other.y; }
                if (other.z > mMax.z) { mMax.z = other.z; }
            }

            glm::vec3 center() {
                return glm::mix(mMin, mMax, 0.5f);
            }

            float width() {
                return mMax.x - mMin.x;
            }

            float height() {
                return mMax.y - mMin.y;
            }

            float depth() {
                return mMax.z - mMin.z;
            }
        };

        const std::vector<glm::vec4> corners = { // screen space receiver box 
            { -1.f,  1.f, depthMin, 1.f }, // corners of near plane
            {  1.f,  1.f, depthMin, 1.f },
            { -1.f, -1.f, depthMin, 1.f },
            {  1.f, -1.f, depthMin, 1.f },
            { -1.f,  1.f,      1.f, 1.f }, // corners of far plane
            {  1.f,  1.f,      1.f, 1.f },
            { -1.f, -1.f,      1.f, 1.f },
            {  1.f, -1.f,      1.f, 1.f }
        };

        glm::mat4 shadowToWorld = glm::inverse(sourceProj * sourceView); // source view pos
        BoundingBox receiverBox;
        for (const auto& corner : corners) {
            glm::vec4 worldPos = shadowToWorld * corner; // transform corners of screen space unit receiver box into source PV space
            worldPos = worldPos / worldPos.w;
            worldPos = worldToLight * worldPos; // rotate corners with light's world rotation
            receiverBox.addNewPosition(worldPos);
        }

        glm::vec3 center = lightToWorld * glm::vec4(receiverBox.center(), 1.f); // receivers center back in light space
        float bias = receiverFrustum.mBias;
        const float boxWidth = receiverBox.width() * 0.5f * (1.f + bias);
        const float boxHeight = receiverBox.height() * 0.5f * (1.f + bias);
        const float boxDepth = receiverBox.depth() * 0.5f * (1.f + bias);

        receiverSpatial.setPosition(center);
        receiverSpatial.setLookDir(lightDir);
        receiverCamera.setOrthoBounds(glm::vec2(-boxWidth, boxWidth), glm::vec2(-boxHeight, boxHeight));
        receiverCamera.setNearFar(-boxDepth, boxDepth);

    }
}
