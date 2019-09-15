#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "FrustumBoundsComponent.hpp"

using namespace neo;

class FrustumBoundsToLineSystem : public System {

public:
    FrustumBoundsToLineSystem() :
        System("FrustumBoundsToLine System")
    {}


    virtual void update(const float dt) override {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto line = camera->getGameObject().getComponentByType<LineMeshComponent>()) {
                if (auto bounds = camera->getGameObject().getComponentByType<FrustumBoundsComponent>()) {

                    glm::vec3 color(1.f);
                    if (auto material = camera->getGameObject().getComponentByType<MaterialComponent>()) {
                        color = material->mDiffuse;
                    }

                    line->clearNodes();

                    // We have to do this ridiculousness because line strip (:
                    line->addNode(bounds->NearLeftBottom, color);
                    line->addNode(bounds->NearLeftTop, color);
                    line->addNode(bounds->NearRightTop, color);
                    line->addNode(bounds->NearRightBottom, color);
                    line->addNode(bounds->NearLeftBottom, color);
                    line->addNode(bounds->FarLeftBottom, color);
                    line->addNode(bounds->FarLeftTop, color);
                    line->addNode(bounds->NearLeftTop, color);
                    line->addNode(bounds->FarLeftTop, color);
                    line->addNode(bounds->FarRightTop, color);
                    line->addNode(bounds->NearRightTop, color);
                    line->addNode(bounds->FarRightTop, color);
                    line->addNode(bounds->FarRightBottom, color);
                    line->addNode(bounds->NearRightBottom, color);
                    line->addNode(bounds->FarRightBottom, color);
                    line->addNode(bounds->FarLeftBottom, color);
                }
            }
        }
    }

};
