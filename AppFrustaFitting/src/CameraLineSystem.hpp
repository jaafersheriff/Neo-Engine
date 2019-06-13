#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "FrustaBoundsComponent.hpp"

using namespace neo;

class CameraLineSystem : public System {

public:
    CameraLineSystem() :
        System("CameraLine System")
    {}


    virtual void update(const float dt) override {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto line = camera->getGameObject().getComponentByType<LineComponent>()) {
                if (auto bounds = camera->getGameObject().getComponentByType<FrustaBoundsComponent>()) {


                    line->clearNodes();

                    // We have to do this ridiculousness because line strip (:
                    line->addNode(bounds->NearLeftBottom);
                    line->addNode(bounds->NearLeftTop);
                    line->addNode(bounds->NearRightTop);
                    line->addNode(bounds->NearRightBottom);
                    line->addNode(bounds->NearLeftBottom);
                    line->addNode(bounds->FarLeftBottom);
                    line->addNode(bounds->FarLeftTop);
                    line->addNode(bounds->NearLeftTop);
                    line->addNode(bounds->FarLeftTop);
                    line->addNode(bounds->FarRightTop);
                    line->addNode(bounds->NearRightTop);
                    line->addNode(bounds->FarRightTop);
                    line->addNode(bounds->FarRightBottom);
                    line->addNode(bounds->NearRightBottom);
                    line->addNode(bounds->FarRightBottom);
                    line->addNode(bounds->FarLeftBottom);
                }

            }
        }

    }

};
