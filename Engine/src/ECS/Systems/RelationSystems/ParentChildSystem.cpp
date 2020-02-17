#include <Engine.hpp>
#include "ParentChildSystem.hpp"

namespace neo {

    void ParentChildSystem::update(const float dt) {

        // Remove parents from children
        for (auto childComponent : Engine::getComponents<ChildComponent>()) {
            bool parentExists = false;
            for (auto engineObject : Engine::getGameObjects()) {
                if (childComponent->parentObject == engineObject) {
                    parentExists = true;
                    break;
                }
            }

            if (!parentExists
                || !childComponent->parentObject->getComponentByType<ParentComponent>()) {
                Engine::removeComponent<ChildComponent>(*childComponent);
            }
        }
    }

}