#include <NeoEngine.hpp>
#include "SinTranslateSystem.hpp"

void SinTranslateSystem::update(const float dt) {
    for (auto comp : NeoEngine::getComponents<SinTranslateComponent>()) {
        double time = Util::getRunTime();
        glm::vec3 oldPos = comp->getGameObject().getSpatial()->mPosition;
        oldPos = comp->mBasePosition + (float)glm::cos(time) * comp->mOffset;
        comp->getGameObject().getSpatial()->setPosition(oldPos);
    }
}
 