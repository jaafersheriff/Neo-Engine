#pragma once

#include "ECS/Systems/System.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectedComponent.hpp"

#include <functional>
#include <glm/glm.hpp>

namespace neo {

    class SelectingSystem : public System {

    public:
        using EntityOperation = std::function<void(ECS&, ECS::Entity entity)>;

        SelectingSystem(
            std::string name,
            EntityOperation reset,
            EntityOperation select,
            EntityOperation edit) :

            System(name),
            mResetOperation(reset),
            mSelectOperation(select),
            mEditorOperation(edit)
        {}

        ~SelectingSystem();


        virtual void init(ECS& ecs) override;
        virtual void update(ECS& ecs) override;
        virtual void imguiEditor(ECS& ecs) override;

    private:
        const EntityOperation mResetOperation; // Called when an object is unselected
        const EntityOperation mSelectOperation; // Called when an object is freshly selected
        const EntityOperation mEditorOperation; // imgui
    };

}
