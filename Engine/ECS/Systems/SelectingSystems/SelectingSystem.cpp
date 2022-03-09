#include "Engine/Engine.hpp"
#include "SelectingSystem.hpp"

#include "ECS/Component/EngineComponents/TagComponent.hpp"

namespace neo {

    void SelectingSystem::init(ECS& _ecs) {
        NEO_UNUSED(_ecs);

        Messenger::addReceiver<ComponentSelectedMessage>(nullptr, [this](const neo::Message& msg, ECS& ecs) {
            const ComponentSelectedMessage & m(static_cast<const ComponentSelectedMessage &>(msg));
            auto oldSelectedEntity = ecs.getView<SelectedComponent>();
            NEO_ASSERT(oldSelectedEntity.size() <= 1, "");
            if (oldSelectedEntity.size() == 1) {
                {
                    MICROPROFILE_SCOPEI("SelectingSystem", "ResetOperation", MP_AUTO);
                    mResetOperation(ecs, oldSelectedEntity.front(), ecs.getComponent<SelectedComponent>(oldSelectedEntity.front()));
                }
                ecs.removeComponent<SelectedComponent>(oldSelectedEntity.front());
            }
            for (auto selectableEntity : ecs.getView<SelectableComponent>()) {
                auto selectable = ecs.getComponent<SelectableComponent>(selectableEntity);
                if (selectable->mID == m.mComponentID) {
                    {
                        MICROPROFILE_SCOPEI("SelectingSystem", "SelectOperation", MP_AUTO);
                        std::string name = ecs.has<TagComponent>(selectableEntity) ? ecs.getComponent<TagComponent>(selectableEntity)->getName() : "";
                        NEO_LOG("New entity selected: %d %s", selectable->mID, name.c_str());
                        mSelectOperation(ecs, selectableEntity, selectable);
                    }
                    ecs.addComponent<SelectedComponent>(selectableEntity);
                }
            }
        });
    }

    void SelectingSystem::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void SelectingSystem::imguiEditor(ECS& ecs) {
        if (auto selected = ecs.getComponent<SelectedComponent>()) {
            MICROPROFILE_SCOPEI("SelectingSystem", "EditorOperation", MP_AUTO);
            mEditorOperation(ecs, selected);
        }
    }

}
