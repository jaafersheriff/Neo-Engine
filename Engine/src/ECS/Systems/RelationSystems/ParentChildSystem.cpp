#include <Engine.hpp>
#include "ParentChildSystem.hpp"

namespace neo {

    void ParentChildSystem::update(const float dt) {

        // Remove children from parents
        for (auto parent : Engine::getComponents<ParentComponent>()) {
            for (int i = 0; i < parent->childrenObjects.size(); i++) {
                GameObject* childObject = parent->childrenObjects[i];

                bool childExists = false;
                for (auto engineObject : Engine::getGameObjects()) {
                    if (childObject == engineObject) {
                        childExists = true;
                        break;
                    }
                }
                if (!childExists 
                    || !childObject->getComponentByType<ChildComponent>() 
                    || childObject->getComponentByType<ChildComponent>()->parentObject->getComponentByType<ParentComponent>() != parent)
                {

                    parent->childrenObjects.erase(parent->childrenObjects.begin() + i);
                    i--;
                }

            }
        }

        // Remove parents from children
        for (auto childComponent : Engine::getComponents<ChildComponent>()) {
            bool parentExists = false;
            for (auto engineObject : Engine::getGameObjects()) {
                if (&childComponent->getGameObject() == engineObject) {
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