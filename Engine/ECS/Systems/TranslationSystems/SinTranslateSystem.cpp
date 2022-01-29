#include "Engine/Engine.hpp"
#include "SinTranslateSystem.hpp"

namespace neo {
    void SinTranslateSystem::update(const float dt) {
        for (auto sin : Engine::getComponents<SinTranslateComponent>()) {
            if (auto spatial = sin->getGameObject().getComponentByType<SpatialComponent>()) {
                double time = Util::getRunTime();
                glm::vec3 oldPos = spatial->getPosition();
                oldPos = sin->mBasePosition + (float)glm::cos(time) * sin->mOffset;
                spatial->setPosition(oldPos);
            }
        }
    }

}