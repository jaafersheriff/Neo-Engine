#include "Engine/Engine.hpp"
#include "SinTranslateSystem.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/TransformationComponent/SinTranslateComponent.hpp"

namespace neo {
    void SinTranslateSystem::update(ECS& ecs) {
        for (auto sin : ecs.getComponents<SinTranslateComponent>()) {
            if (auto spatial = sin->getGameObject().getComponentByType<SpatialComponent>()) {
                if (auto stats = ecs.getSingleComponent<FrameStatsComponent>()) {
                    double time = stats->mRunTime;
                    glm::vec3 oldPos = spatial->getPosition();
                    oldPos = sin->mBasePosition + (float)glm::cos(time) * sin->mOffset;
                    spatial->setPosition(oldPos);
                }
            }
        }
    }

}