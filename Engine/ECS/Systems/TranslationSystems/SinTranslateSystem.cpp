#include "SinTranslateSystem.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/SinTranslateComponent.hpp"

namespace neo {
    void SinTranslateSystem::update(ECS& ecs) {

        if (auto frameStats = ecs.getComponent<FrameStatsComponent>()) {

            for (auto& tuple : ecs.getComponentTuples<SinTranslateComponent, SpatialComponent>()) {
                auto& [sin, spatial] = tuple.get();
                double time = frameStats->mRunTime;
                glm::vec3 oldPos = spatial.getPosition();
                oldPos = sin.mBasePosition + (float)glm::cos(time) * sin.mOffset;
                spatial.setPosition(oldPos);
            }
        }
    }

}