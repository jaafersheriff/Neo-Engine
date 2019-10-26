#include <Engine.hpp>
#include "SinTranslateSystem.hpp"

namespace neo {
    void SinTranslateSystem::update(const float dt) {
        for (auto sinComps : Engine::getComponentTuples<SinTranslateComponent, SpatialComponent>()) {
            auto sin = sinComps.get<SinTranslateComponent>();
            auto spatial = sinComps.get<SpatialComponent>();

            double time = Util::getRunTime();
            glm::vec3 oldPos = spatial->getPosition();
            oldPos = sin->mBasePosition + (float)glm::cos(time) * sin->mOffset;
            spatial->setPosition(oldPos);
        }
    }

}