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
        Dumb,
        Naive,
        A
    } method = Naive;

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
    };

    void methodDumb(CameraComponent* perspectiveCamera, SpatialComponent* perspectiveSpat, FrustumBoundsComponent* perspectiveBounds,
            CameraComponent* orthoCamera, SpatialComponent* orthoSpat, MockOrthoComponent* mockOrthoCamera) {
        glm::vec3 source = orthoSpat->getPosition();
        glm::vec3 destination = perspectiveSpat->getPosition();
        orthoCamera->setLookDir(destination - source);
        orthoSpat->setPosition(destination - orthoCamera->getLookDir() * mockOrthoCamera->distance);
    }

    void methodNaive(CameraComponent* perspectiveCamera, SpatialComponent* perspectiveSpat, FrustumBoundsComponent* perspectiveBounds, 
               CameraComponent* orthoCamera, SpatialComponent* orthoSpat, MockOrthoComponent* mockOrthoCamera) {
        BoundingBox box(perspectiveBounds);
        glm::vec3 dif = (box.max - box.min);
        float diflen = glm::length(dif);

        glm::vec3 center = perspectiveSpat->getPosition() + perspectiveCamera->getLookDir() * perspectiveCamera->getNearFar().x + perspectiveCamera->getLookDir() * (perspectiveCamera->getNearFar().y - perspectiveCamera->getNearFar().x) / 2.f;

        // TODO - this is broken -- treating it as point light when it should be a directional light
        //      this will need its own phong shadow shader that uses a directional light
        orthoSpat->setPosition(center - orthoCamera->getLookDir() * mockOrthoCamera->distance);

        glm::vec3 nearPos = center - orthoCamera->getLookDir() * diflen / 2.f;
        float near = glm::distance(orthoSpat->getPosition(), nearPos);
        orthoCamera->setNearFar(near, near + diflen);
        orthoCamera->setOrthoBounds(glm::vec2(-diflen / 2.f, diflen / 2.f), glm::vec2(-diflen / 2.f, diflen / 2.f));
    }

    void methodA(CameraComponent* perspectiveCamera, SpatialComponent* perspectiveSpat, FrustumBoundsComponent* perspectiveBounds, 
               CameraComponent* orthoCamera, SpatialComponent* orthoSpat, MockOrthoComponent* mockOrthoCamera) {
        // transform frustum corners to light's local space

        glm::vec3 dummyNearLeftBottom  = perspectiveBounds->NearLeftBottom;
        glm::vec3 dummyNearLeftTop     = perspectiveBounds->NearLeftTop;
        glm::vec3 dummyNearRightBottom = perspectiveBounds->NearRightBottom;
        glm::vec3 dummyNearRightTop    = perspectiveBounds->NearRightTop;
        glm::vec3 dummyFarLeftBottom   = perspectiveBounds->FarLeftBottom;
        glm::vec3 dummyFarLeftTop      = perspectiveBounds->FarLeftTop;
        glm::vec3 dummyFarRightBottom  = perspectiveBounds->FarRightBottom;
        glm::vec3 dummyFarRightTop     = perspectiveBounds->FarRightTop;

        // mat4 invview
        // Notes - finds the bounding box in light space
        {
            glm::mat4 M = glm::inverse(orthoSpat->getModelMatrix());

//             perspectiveBounds->NearLeftBottom  = glm::vec3(M * glm::vec4(dummyNearLeftBottom, 1.f));
//             perspectiveBounds->NearLeftTop     = glm::vec3(M * glm::vec4(dummyNearLeftTop, 1.f));
//             perspectiveBounds->NearRightBottom = glm::vec3(M * glm::vec4(dummyNearRightBottom, 1.f));
//             perspectiveBounds->NearRightTop    = glm::vec3(M * glm::vec4(dummyNearRightTop, 1.f));
//             perspectiveBounds->FarLeftBottom   = glm::vec3(M * glm::vec4(dummyFarLeftBottom, 1.f));
//             perspectiveBounds->FarLeftTop      = glm::vec3(M * glm::vec4(dummyFarLeftTop, 1.f));
//             perspectiveBounds->FarRightBottom  = glm::vec3(M * glm::vec4(dummyFarRightBottom, 1.f));
//             perspectiveBounds->FarRightTop     = glm::vec3(M * glm::vec4(dummyFarRightTop, 1.f));

            dummyNearLeftBottom  = glm::vec3(M * glm::vec4(dummyNearLeftBottom, 1.f));
            dummyNearLeftTop     = glm::vec3(M * glm::vec4(dummyNearLeftTop, 1.f));
            dummyNearRightBottom = glm::vec3(M * glm::vec4(dummyNearRightBottom, 1.f));
            dummyNearRightTop    = glm::vec3(M * glm::vec4(dummyNearRightTop, 1.f));
            dummyFarLeftBottom   = glm::vec3(M * glm::vec4(dummyFarLeftBottom, 1.f));
            dummyFarLeftTop      = glm::vec3(M * glm::vec4(dummyFarLeftTop, 1.f));
            dummyFarRightBottom  = glm::vec3(M * glm::vec4(dummyFarRightBottom, 1.f));
            dummyFarRightTop     = glm::vec3(M * glm::vec4(dummyFarRightTop, 1.f));
        }

        // get bounding box for transformed corners
        BoundingBox box;
        box.addNewPosition(dummyNearLeftBottom);
        box.addNewPosition(dummyNearLeftTop);
        box.addNewPosition(dummyNearRightBottom);
        box.addNewPosition(dummyNearRightTop);
        box.addNewPosition(dummyFarLeftBottom);
        box.addNewPosition(dummyFarLeftTop);
        box.addNewPosition(dummyFarRightBottom);
        box.addNewPosition(dummyFarRightTop);


                // just visualize transformations for now
        if (auto orthoBounds = orthoSpat->getGameObject().getComponentByType<FrustumBoundsComponent>()) {
            // orthoBounds->NearLeftBottom = glm::vec3(box.min);
            // orthoBounds->NearLeftTop = glm::vec3(box.min.x, box.max.y, box.min.z);
            // orthoBounds->NearRightBottom = glm::vec3(box.max.x, box.min.y, box.min.z);
            // orthoBounds->NearRightTop = glm::vec3(box.max.x, box.max.y, box.min.z);
            // orthoBounds->FarLeftBottom = glm::vec3(box.min.x, box.min.y, box.max.z);
            // orthoBounds->FarLeftTop = glm::vec3(box.min.x, box.max.y, box.max.z);
            // orthoBounds->FarRightBottom = glm::vec3(box.max.x, box.min.y, box.max.z);
            // orthoBounds->FarRightTop = glm::vec3(box.max);
        }


        // // set ortho camera extents to be the bounding box
        glm::vec3 center = perspectiveSpat->getPosition() + perspectiveCamera->getLookDir() * perspectiveCamera->getNearFar().x + perspectiveCamera->getLookDir() * (perspectiveCamera->getNearFar().y - perspectiveCamera->getNearFar().x) / 2.f;
        orthoCamera->setOrthoBounds(glm::vec2(box.min.x, box.max.x), 
        glm::vec2(box.min.y, box.max.y));
        orthoSpat->setPosition(center - orthoCamera->getLookDir() * mockOrthoCamera->distance);

        // // this is the only part thats broken
        glm::vec3 nearPos = center - orthoCamera->getLookDir() * (box.max.z-box.min.z) / 2.f;
        float near = glm::distance(orthoSpat->getPosition(), nearPos);
        float centerdist = glm::distance(orthoSpat->getPosition(), center);
        orthoCamera->setNearFar(centerdist- (box.max.z-box.min.z)/2.f, centerdist + (box.max.z-box.min.z)/2.f);
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
            if (auto perspectiveBounds = perspectiveCamera->getGameObject().getComponentByType<FrustumBoundsComponent>()) {
                switch (method) {
                    case Dumb:
                        methodDumb(perspectiveCamera, perspectiveSpat, perspectiveBounds,
                            orthoCamera, orthoSpat, mockOrthoCamera);
                        break;
                    case A:
                        methodA(perspectiveCamera, perspectiveSpat, perspectiveBounds,
                            orthoCamera, orthoSpat, mockOrthoCamera);
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
