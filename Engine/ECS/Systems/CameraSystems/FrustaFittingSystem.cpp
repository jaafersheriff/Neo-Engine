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

#include <algorithm>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>

namespace neo {

    void FrustaFittingSystem::update(ECS& ecs) {
        auto sourceCamera = ecs.getComponentTuple<FrustumFitSourceComponent, SpatialComponent>();
        auto receiverCamera = ecs.getComponentTuple<FrustumFitReceiverComponent, SpatialComponent>();
        auto light = ecs.getComponentTuple<LightComponent, SpatialComponent>();
        if (!receiverCamera || !sourceCamera || !light) {
            return;
        }
        OrthoCameraComponent* orthoCamera = ecs.getComponent<OrthoCameraComponent>(receiverCamera.mEntity);
        PerspectiveCameraComponent* perspectiveCamera = ecs.getComponent<PerspectiveCameraComponent>(sourceCamera.mEntity);
        if (!orthoCamera || !perspectiveCamera) {
            return;
        }

        /////////////////////// Do the fitting! ///////////////////////////////
        auto& orthoSpat = receiverCamera.get<SpatialComponent>();
        auto& perspectiveSpat = sourceCamera.get<SpatialComponent>();
        NEO_UNUSED(perspectiveSpat);
        auto& lightSpat = light.get<SpatialComponent>();

        const glm::vec3 lightDir = lightSpat.getLookDir();
        const glm::vec3 up = lightSpat.getUpDir();

        const auto& sceneView = perspectiveSpat.getView();
        const auto& sceneProj = perspectiveCamera->getProj();
        const auto& worldToLight = glm::lookAt(lightDir, glm::vec3(0.f), up);
        const auto& lightToWorld = glm::inverse(worldToLight);

        // const float aspect = sceneProj[1][1] / sceneProj[0][0];
        // const float fov = 2.f * glm::atan(1.f / sceneProj[1][1]);
        // const float zNear = sceneProj[3][2] / (sceneProj[2][2] - 1.f);
        // const float zFar = sceneProj[3][2] / (sceneProj[2][2] + 1.f);
        // const float zRange = zFar - zNear;
        const float depthMin = -1.f; // GL things

        struct BoundingBox {
            glm::vec3 mMin = glm::vec3(std::numeric_limits<float>::max());
            glm::vec3 mMax = glm::vec3(-std::numeric_limits<float>::max());
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
                if (other.x < mMin.x) { mMin.x = other.x; }
                if (other.y < mMin.y) { mMin.y = other.y; }
                if (other.z < mMin.z) { mMin.z = other.z; }
                if (other.x > mMax.x) { mMax.x = other.x; }
                if (other.y > mMax.y) { mMax.y = other.y; }
                if (other.z > mMax.z) { mMax.z = other.z; }
                points.push_back(other);
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
        float bias = receiverCamera.get<FrustumFitReceiverComponent>().mBias;
        const float boxWidth = orthoBox.width() * 0.5f * (1.f + bias);
        const float boxHeight = orthoBox.height() * 0.5f * (1.f + bias);
        const float boxDepth = orthoBox.depth() * 0.5f * (1.f + bias);

        orthoSpat.setPosition(center);
        orthoSpat.setLookDir(lightDir);
        orthoCamera->setOrthoBounds(glm::vec2(-boxWidth, boxWidth), glm::vec2(-boxHeight, boxHeight));
        orthoCamera->setNearFar(-boxDepth, boxDepth);

    }
}
