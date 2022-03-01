#include "Engine/Engine.hpp"
#include "SelectingSystem.hpp"

#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectedComponent.hpp"

namespace neo {

    void SelectingSystem::init(ECS& _ecs) {
        NEO_UNUSED(_ecs);

        // It's only possible to have one SelectedSystem at a time..
        Messenger::addReceiver<ComponentSelectedMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            const ComponentSelectedMessage & m(static_cast<const ComponentSelectedMessage &>(msg));
            if (auto oldSelected = ecs.getSingleComponent<SelectedComponent>()) {
                {
                    MICROPROFILE_SCOPEI("SelectingSystem", "ResetOperation", MP_AUTO);
                    mResetOperation(ecs, oldSelected);
                }
                ecs.removeComponent(*oldSelected);
            }
            for (auto selectable : ecs.getComponents<SelectableComponent>()) {
                if (selectable->mID == m.mComponentID) {
                    {
                        MICROPROFILE_SCOPEI("SelectingSystem", "SelectOperation", MP_AUTO);
                        mSelectOperation(ecs, selectable);
                    }
                    ecs.addComponent<SelectedComponent>(&selectable->getGameObject());
                }
            }
        });
    }

    void SelectingSystem::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void SelectingSystem::imguiEditor(ECS& ecs) {
        auto selected = ecs.getSingleComponent<SelectedComponent>();
        if (selected) {
            MICROPROFILE_SCOPEI("SelectingSystem", "EditorOperation", MP_AUTO);
            mEditorOperation(ecs, selected);
        }
    }

}
