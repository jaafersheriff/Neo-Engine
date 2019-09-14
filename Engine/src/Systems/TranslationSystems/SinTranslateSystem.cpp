#include <Engine.hpp>
#include "SinTranslateSystem.hpp"

namespace neo {
    void SinTranslateSystem::update(const float dt) {
        for (auto comp : Engine::getComponents<SinTranslateComponent>()) {
            if (auto spatial = comp->getGameObject().getComponentByType<SpatialComponent>()) {
                double time = Util::getRunTime();
                glm::vec3 oldPos = spatial->getPosition();
                oldPos = comp->mBasePosition + (float)glm::cos(time) * comp->mOffset;
                spatial->setPosition(oldPos);
            }
        }
    }

}