#include "SinTranslateSystem.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/SinTranslateComponent.hpp"

namespace neo {
    void SinTranslateSystem::update(ECS& ecs) {

        auto stats = ecs.getComponent<FrameStatsComponent>();
        ecs.getView<SinTranslateComponent>().each([&ecs, &stats](ECS::Entity entity, SinTranslateComponent& sin) {
            auto& spatial = ecs.getComponent<SpatialComponent>(entity);
            double time = stats.mRunTime;
            glm::vec3 oldPos = spatial.getPosition();
            oldPos = sin.mBasePosition + (float)glm::cos(time) * sin.mOffset;
            spatial.setPosition(oldPos);
        });
    }

}