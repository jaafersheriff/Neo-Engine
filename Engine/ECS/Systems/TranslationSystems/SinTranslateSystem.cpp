#include "SinTranslateSystem.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/SinTranslateComponent.hpp"

namespace neo {
    void SinTranslateSystem::update(ECS& ecs) {
        TRACY_ZONEN("SinTranslateSystem");

        if (auto frameStatsOpt = ecs.getComponent<FrameStatsComponent>()) {
            auto&& [_, frameStats] = *frameStatsOpt;

            for (auto&& [entity, sin, spatial] : ecs.getView<SinTranslateComponent, SpatialComponent>().each()) {
                double time = frameStats.mRunTime;
                glm::vec3 oldPos = spatial.getPosition();
                oldPos = sin.mBasePosition + (float)glm::cos(time) * sin.mOffset;
                spatial.setPosition(oldPos);
            }
        }
    }

}