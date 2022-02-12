#include "Engine/Engine.hpp"
#include "SelectingSystem.hpp"

#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectedComponent.hpp"

namespace neo {

    void SelectingSystem::init() {
        Messenger::addReceiver<ComponentSelectedMessage>(nullptr, [&](const neo::Message& msg, ECS& ecs) {
            const ComponentSelectedMessage & m(static_cast<const ComponentSelectedMessage &>(msg));
            if (auto oldSelected = ecs.getSingleComponent<SelectedComponent>()) {
                {
                    MICROPROFILE_SCOPEI("SelectingSystem", "ResetOperation", MP_AUTO);
                    mResetOperation(ecs, oldSelected);
                }
                ecs.removeComponent(*oldSelected);
            }
            for (auto selectable : ecs.getComponents<SelectableComponent>()) {
                if (selectable->mID == m.componentID) {
                    {
                        MICROPROFILE_SCOPEI("SelectingSystem", "SelectOperation", MP_AUTO);
                        mSelectOperation(ecs, selectable);
                    }
                    ecs.addComponent<SelectedComponent>(&selectable->getGameObject());
                    break;
                }
            }
        });
    }

    void SelectingSystem::update(ECS& ecs) {
    }

    void SelectingSystem::imguiEditor(ECS& ecs) {
        auto selected = ecs.getSingleComponent<SelectedComponent>();
        if (selected) {
            MICROPROFILE_SCOPEI("SelectingSystem", "EditorOperation", MP_AUTO);
            mEditorOperation(ecs, selected);
        }
    }

}
