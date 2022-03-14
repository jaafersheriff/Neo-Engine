#include "Engine/Engine.hpp"
#include "SelectingSystem.hpp"

#include "ECS/Messaging/Messenger.hpp"

#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {
    namespace {
        void _onEntitySelected(ECS& ecs, const EntitySelectedMessage& msg) {
            if (auto oldSelectedEntity = ecs.getSingleView<SelectedComponent, SpatialComponent>()) {
                {
                    MICROPROFILE_SCOPEI("SelectingSystem", "ResetOperation", MP_AUTO);
                    // mResetOperation(ecs, std::get<0>(*oldSelectedEntity));
                }
                ecs.removeComponent<SelectedComponent>(std::get<0>(*oldSelectedEntity));
            }
            auto selectableView = ecs.getView<SelectableComponent, SpatialComponent>();
            NEO_ASSERT(selectableView.find(static_cast<ECS::Entity>(msg.mEntity)) != selectableView.end(), "Selected entity doesn't have a selectable");
            {
                MICROPROFILE_SCOPEI("SelectingSystem", "SelectOperation", MP_AUTO);
                std::string name = ecs.has<TagComponent>(msg.mEntity) ? ecs.getComponent<TagComponent>(msg.mEntity)->getName() : "";
                NEO_LOG("New entity selected: %d %s", msg.mEntity, name.c_str());
                // mSelectOperation(ecs, msg.mEntity);
            }
            ecs.addComponent<SelectedComponent>(msg.mEntity);
        }
    }

    void SelectingSystem::init(ECS& ecs) {

        Messenger::addReceiver<EntitySelectedMessage, &_onEntitySelected>(ecs);
    }

    void SelectingSystem::update(ECS& ecs) {
        NEO_UNUSED(ecs);
    }

    void SelectingSystem::imguiEditor(ECS& ecs) {
        if (auto selectedView = ecs.getComponent<SelectedComponent>()) {
            auto&& [selectedEntity, selected] = *selectedView;
            MICROPROFILE_SCOPEI("SelectingSystem", "EditorOperation", MP_AUTO);
            mEditorOperation(ecs, selectedEntity);
        }
    }

}
