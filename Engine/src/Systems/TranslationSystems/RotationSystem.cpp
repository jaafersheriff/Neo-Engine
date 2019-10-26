#include <Engine.hpp>
#include "RotationSystem.hpp"

namespace neo {

    void RotationSystem::update(const float dt) {
        for (auto rotComps : Engine::getComponentTuples<RotationComponent, SpatialComponent>()) {
            auto rotation = rotComps.get<RotationComponent>();
            auto spatial = rotComps.get<SpatialComponent>();
            glm::mat4 R(1.f);
            R *= glm::rotate(glm::mat4(1.f), dt * rotation->mSpeed.x, glm::vec3(1, 0, 0));
            R *= glm::rotate(glm::mat4(1.f), dt * rotation->mSpeed.y, glm::vec3(0, 1, 0));
            R *= glm::rotate(glm::mat4(1.f), dt * rotation->mSpeed.z, glm::vec3(0, 0, 1));
            spatial->rotate(glm::mat3(R));
        }
    }

}