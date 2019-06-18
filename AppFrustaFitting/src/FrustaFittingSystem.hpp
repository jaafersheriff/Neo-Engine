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

    enum Method {
        Naive,
        A
    } method = Naive;

    void methodNaive(const CameraComponent* perspectiveCamera, const SpatialComponent* perspectiveSpat, const FrustaBoundsComponent* perspectiveBounds, 
               CameraComponent* orthoCamera, SpatialComponent* orthoSpat, MockOrthoComponent* mockOrthoCamera) {
        FrustaBoundsComponent::BoundingBox box = perspectiveBounds->getBoundingBox();
        glm::vec3 dif = (box.max - box.min);
        float diflen = glm::length(dif);

        glm::vec3 center = perspectiveSpat->getPosition() + perspectiveCamera->getLookDir() * (perspectiveCamera->getNearFar().y - perspectiveCamera->getNearFar().x) / 2.f;

        // TODO - this is broken -- treating it as point light when it should be a directional light
        //      this will need its own phong shadow shader that uses a directional light
        orthoSpat->setPosition(center - orthoCamera->getLookDir() * mockOrthoCamera->distance);

        glm::vec3 nearPos = center - orthoCamera->getLookDir() * diflen / 2.f;
        float near = glm::distance(orthoSpat->getPosition(), nearPos);
        orthoCamera->setNearFar(near, near + diflen);
        orthoCamera->setOrthoBounds(glm::vec2(-diflen / 2.f, diflen / 2.f), glm::vec2(-diflen / 2.f, diflen / 2.f));
    }

    void methodA(FrustaBoundsComponent* perspectiveBounds, SpatialComponent* orthoSpat, CameraComponent* orthoCamera) {
        // transform frustum corners to light's local space
        glm::vec3 dummyNearLeftBottom  = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->NearLeftBottom, 1.f));
        glm::vec3 dummyNearLeftTop     = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->NearLeftTop, 1.f));
        glm::vec3 dummyNearRightBottom = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->NearRightBottom, 1.f));
        glm::vec3 dummyNearRightTop    = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->NearRightTop, 1.f));
        glm::vec3 dummyFarLeftBottom   = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->FarLeftBottom, 1.f));
        glm::vec3 dummyFarLeftTop      = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->FarLeftTop, 1.f));
        glm::vec3 dummyFarRightBottom  = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->FarRightBottom, 1.f));
        glm::vec3 dummyFarRightTop     = glm::vec3(glm::inverse(orthoCamera->getView()) * glm::vec4(perspectiveBounds->FarRightTop, 1.f));

        // TODO -remove this is just for testing
        perspectiveBounds->NearLeftBottom  = dummyNearLeftBottom;
        perspectiveBounds->NearLeftTop     = dummyNearLeftTop;
        perspectiveBounds->NearRightBottom = dummyNearRightBottom;
        perspectiveBounds->NearRightTop    = dummyNearRightTop;
        perspectiveBounds->FarLeftBottom   = dummyFarLeftBottom;
        perspectiveBounds->FarLeftTop      = dummyFarLeftTop;
        perspectiveBounds->FarRightBottom  = dummyFarRightBottom;
        perspectiveBounds->FarRightTop     = dummyFarRightTop;
        orthoCamera->setNearFar(0.1f, 10.f);
        orthoCamera->setOrthoBounds(glm::vec2(-4.f, 4.f), glm::vec2(-4.f, 4.f));

        // TODO - get bounding box for transformed corners
        // TODO - set ortho camera extents to be the bounding box
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
            perspectiveCamera->setLookDir(glm::vec3(f, f/2, g));
        }

        if (updateOrtho) {
            if (auto perspectiveBounds = perspectiveCamera->getGameObject().getComponentByType<FrustaBoundsComponent>()) {
                switch (method) {
                    case A:
                        methodA(perspectiveBounds, orthoSpat, orthoCamera);
                        break;
                    case Naive:
                    default:
                        methodNaive(perspectiveCamera, perspectiveSpat, perspectiveBounds,
                            orthoCamera, orthoSpat, mockOrthoCamera);

                }
            }
        }
    }
};
