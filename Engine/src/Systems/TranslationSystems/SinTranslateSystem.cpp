#include <Engine.hpp>
#include "SinTranslateSystem.hpp"

namespace neo {
    void SinTranslateSystem::update(const float dt) {
        for (auto comp : Engine::getComponents<SinTranslateComponent>()) {
            double time = Util::getRunTime();
            glm::vec3 oldPos = comp->getGameObject().getSpatial()->getPosition();
            oldPos = comp->mBasePosition + (float)glm::cos(time) * comp->mOffset;
            comp->getGameObject().getSpatial()->setPosition(oldPos);
        }
    }

}