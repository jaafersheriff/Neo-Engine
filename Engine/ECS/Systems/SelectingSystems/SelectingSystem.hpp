#pragma once

#include "ECS/Systems/System.hpp"
#include <functional>

#include "glm/glm.hpp"

#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectedComponent.hpp"

namespace neo {

    class SelectingSystem : public System {

    public:
        using SelectableOperation = std::function<void(ECS&, SelectableComponent*)>;
        using SelectedOperation = std::function<void(ECS&, SelectedComponent*)>;

        SelectingSystem(
            std::string name,
            SelectedOperation reset,
            SelectableOperation select,
            SelectedOperation edit) :

            System(name),
            mResetOperation(reset),
            mSelectOperation(select),
            mEditorOperation(edit)
        {}


        virtual void init(ECS& ecs) override;
        virtual void update(ECS& ecs) override;
        virtual void imguiEditor(ECS& ecs) override;

    private:
        const SelectedOperation mResetOperation; // Called when an object is unselected
        const SelectableOperation mSelectOperation; // Called when an object is freshly selected
        const SelectedOperation mEditorOperation; // imgui
    };

}