#include "Engine/Engine.hpp"
#include "RotationSystem.hpp"

namespace neo {

    void RotationSystem::update(ECS& ecs) {
        for (auto rotation : ecs.getComponents<RotationComponent>()) {
            if (auto spatial = rotation->getGameObject().getComponentByType<SpatialComponent>()) {
                if (auto frameStats = ecs.getSingleComponent<FrameStatsComponent>()) {
                    glm::mat4 R(1.f);
                    R *= glm::rotate(glm::mat4(1.f), frameStats->mDT * rotation->mSpeed.x, glm::vec3(1, 0, 0));
                    R *= glm::rotate(glm::mat4(1.f), frameStats->mDT * rotation->mSpeed.y, glm::vec3(0, 1, 0));
                    R *= glm::rotate(glm::mat4(1.f), frameStats->mDT * rotation->mSpeed.z, glm::vec3(0, 0, 1));
                    spatial->rotate(glm::mat3(R));
                }
            }
        }
    }
}