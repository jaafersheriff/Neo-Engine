#include <NeoEngine.hpp>
#include "RotationSystem.hpp"

void RotationSystem::update(const float dt) {
    for (auto comp : NeoEngine::getComponents<RotationComponent>()) {
        glm::mat4 R(1.f);
        R *= glm::rotate(glm::mat4(1.f), dt * comp->mSpeed.x, glm::vec3(1, 0, 0));
        R *= glm::rotate(glm::mat4(1.f), dt * comp->mSpeed.y, glm::vec3(0, 1, 0));
        R *= glm::rotate(glm::mat4(1.f), dt * comp->mSpeed.z, glm::vec3(0, 0, 1));
        comp->getGameObject().getSpatial()->rotate(glm::mat3(R));
    }
}
 