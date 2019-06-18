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

    void methodNaive(CameraComponent* perspectiveCamera, SpatialComponent* perspectiveSpat, FrustaBoundsComponent* perspectiveBounds, 
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

    void methodA() {
        std::cout << "A" << std::endl;
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
                        methodA();
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
