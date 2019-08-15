#pragma once

#include <Engine.hpp>
#include "Systems/System.hpp"

#include "SnowComponent.hpp"

using namespace neo;

class SnowSystem : public System {

    public:
        SnowSystem() :
            System("Snow System")
        {}

        virtual void update(const float dt) override {
            for (auto comp : Engine::getComponents<SnowComponent>()) {
                comp->height = -0.19f * comp->snowSize + 0.17f;

                comp->snowAngle = comp->getGameObject().getSpatial()->getUpDir();
                comp->snowAngle.x = -comp->snowAngle.x;
                comp->snowAngle.z = -comp->snowAngle.z;

                // TODO - messaging...
                auto line = comp->getGameObject().getComponentByType<LineComponent>();
                if (line) {
                    line->removeNode(1);
                    line->addNode(comp->snowAngle);
                }

            }
        }
};