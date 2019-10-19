#include "EditorSystem.hpp"
#include <Engine.hpp>

namespace neo {

    EditorSystem::EditorSystem() 
        : SelectingSystem(
            25,
            100.f,
            [](SelectedComponent*) { return true; },
            [](SelectableComponent*) {},
            [](SelectedComponent* selected, glm::vec3 mousePos) {
                // Move selected object to moise pos
                if (auto spatial = selected->getGameObject().getComponentByType<SpatialComponent>()) {
                    spatial->setPosition(mousePos);
                }
                // Add lines
                auto& xLine = Engine::addComponent<LineMeshComponent>(&selected->getGameObject(), glm::vec3(1, 0, 0));
                xLine.addNode(glm::vec3(0.f));
                xLine.addNode(glm::vec3(1.f, 0.f, 0.f));
                auto& yLine = Engine::addComponent<LineMeshComponent>(&selected->getGameObject(), glm::vec3(0, 1, 0));
                yLine.addNode(glm::vec3(0.f));
                yLine.addNode(glm::vec3(0.f, 1.f, 0.f));
                auto& zLine = Engine::addComponent<LineMeshComponent>(&selected->getGameObject(), glm::vec3(0, 0, 1));
                zLine.addNode(glm::vec3(0.f));
                zLine.addNode(glm::vec3(0.f, 0.f, 1.f));

                xLine.mUseParentSpatial = yLine.mUseParentSpatial = zLine.mUseParentSpatial = true;
                xLine.mWriteDepth = yLine.mWriteDepth = zLine.mWriteDepth = false;
            }) 
    { }
}
