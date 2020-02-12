#pragma once

#include <Engine.hpp>
#include "ECS/Systems/System.hpp"

#include "SnowComponent.hpp"

using namespace neo;

class SnowSystem : public System {

    public:
        SnowSystem() :
            System("Snow System")
        {}

        virtual void update(const float dt) override {
            for (auto comp : Engine::getComponents<SnowComponent>()) {
                comp->mHeight = -0.19f * comp->mSnowSize + 0.17f;

                comp->mSnowAngle = comp->getGameObject().getComponentByType<SpatialComponent>()->getUpDir();
                comp->mSnowAngle.x = -comp->mSnowAngle.x;
                comp->mSnowAngle.z = -comp->mSnowAngle.z;

                // TODO - messaging...
                auto line = comp->getGameObject().getComponentByType<LineMeshComponent>();
                if (line) {
                    line->removeNode(1);
                    line->addNode(comp->mSnowAngle);
                }

            }
        }
};