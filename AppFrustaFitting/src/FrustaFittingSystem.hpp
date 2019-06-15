#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include <limits>

using namespace neo;

class FrustaFittingSystem : public System {

public:
    FrustaFittingSystem() :
        System("FrustaFitting System")
    {}

    bool updatePerspective = true;
    bool updateOrtho = true;

    void getMinMax(glm::vec3& min, glm::vec3& max, glm::vec3 other) {
        if (other.x < min.x) { min.x = other.x; }
        if (other.y < min.y) { min.y = other.y; }
        if (other.z < min.z) { min.z = other.z; }
        if (other.x > max.x) { max.x = other.x; }
        if (other.y > max.y) { max.y = other.y; }
        if (other.z > max.z) { max.z = other.z; }
    }

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
            perspectiveCamera->setLookDir(glm::vec3(f, 0.f, g));
        }

        if (updateOrtho) {
            if (auto perspectiveBounds = perspectiveCamera->getGameObject().getComponentByType<FrustaBoundsComponent>()) {
                glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
                glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());
                getMinMax(min, max, perspectiveBounds->FarLeftBottom - perspectiveSpat->getPosition());
                getMinMax(min, max, perspectiveBounds->FarLeftTop - perspectiveSpat->getPosition());
                getMinMax(min, max, perspectiveBounds->FarRightBottom - perspectiveSpat->getPosition());
                getMinMax(min, max, perspectiveBounds->FarRightTop - perspectiveSpat->getPosition());
                getMinMax(min, max, perspectiveBounds->NearLeftBottom - perspectiveSpat->getPosition());
                getMinMax(min, max, perspectiveBounds->NearLeftTop - perspectiveSpat->getPosition());
                getMinMax(min, max, perspectiveBounds->NearRightBottom - perspectiveSpat->getPosition());
                getMinMax(min, max, perspectiveBounds->NearRightTop - perspectiveSpat->getPosition());
                glm::vec3 dif = (max - min);
                float diflen = glm::length(dif);

                glm::vec3 center = perspectiveSpat->getPosition() + perspectiveCamera->getLookDir() * (perspectiveCamera->getNearFar().y - perspectiveCamera->getNearFar().x) / 2.f;

                orthoSpat->setPosition(center - orthoCamera->getLookDir() * mockOrthoCamera->distance);

                glm::vec3 nearPos = center - orthoCamera->getLookDir() * dif / 2.f;
                float near = glm::distance(orthoSpat->getPosition(), nearPos);
                orthoCamera->setNearFar(near, near + diflen);
                orthoCamera->setOrthoBounds(glm::vec2(-diflen / 2.f, diflen / 2.f), glm::vec2(-diflen / 2.f, diflen / 2.f));
            }
        }
    }
};
