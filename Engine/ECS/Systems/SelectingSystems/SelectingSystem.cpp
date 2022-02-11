#include "Engine/Engine.hpp"
#include "SelectingSystem.hpp"

#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectedComponent.hpp"

namespace neo {

    void SelectingSystem::init() {
        Messenger::addReceiver<ComponentSelectedMessage>(nullptr, [&](const neo::Message& msg) {
            const ComponentSelectedMessage & m(static_cast<const ComponentSelectedMessage &>(msg));
            if (auto oldSelected = Engine::getSingleComponent<SelectedComponent>()) {
                {
                    MICROPROFILE_SCOPEI("SelectingSystem", "ResetOperation", MP_AUTO);
                    mResetOperation(oldSelected);
                }
                Engine::removeComponent(*oldSelected);
            }
            for (auto selectable : Engine::getComponents<SelectableComponent>()) {
                if (selectable->mID == m.componentID) {
                    {
                        MICROPROFILE_SCOPEI("SelectingSystem", "SelectOperation", MP_AUTO);
                        mSelectOperation(selectable);
                    }
                    Engine::addComponent<SelectedComponent>(&selectable->getGameObject());
                    break;
                }
            }
        });
    }

    void SelectingSystem::update(const float dt) {
        NEO_UNUSED(dt);
    }

    void SelectingSystem::imguiEditor() {
        auto selected = Engine::getSingleComponent<SelectedComponent>();
        if (selected) {
            MICROPROFILE_SCOPEI("SelectingSystem", "EditorOperation", MP_AUTO);
            mEditorOperation(selected);
        }
    }

}
