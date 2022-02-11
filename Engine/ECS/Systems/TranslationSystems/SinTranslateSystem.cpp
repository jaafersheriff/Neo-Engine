#include "Engine/Engine.hpp"
#include "SinTranslateSystem.hpp"

namespace neo {
    void SinTranslateSystem::update(const float dt) {
        NEO_UNUSED(dt);
        for (auto sin : Engine::getComponents<SinTranslateComponent>()) {
            if (auto spatial = sin->getGameObject().getComponentByType<SpatialComponent>()) {
                if (auto stats = Engine::getSingleComponent<FrameStatsComponent>()) {
                    double time = stats->mRunTime;
                    glm::vec3 oldPos = spatial->getPosition();
                    oldPos = sin->mBasePosition + (float)glm::cos(time) * sin->mOffset;
                    spatial->setPosition(oldPos);
                }
            }
        }
    }

}