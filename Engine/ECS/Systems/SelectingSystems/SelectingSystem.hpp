#pragma once

#include "ECS/Systems/System.hpp"
#include <functional>

#include "glm/glm.hpp"

namespace neo {

    class SelectedComponent;
    class SelectableComponent;
    class MouseRayComponent;

    class SelectingSystem : public System {

    public:
        using SelectableOperation = std::function<void(SelectableComponent*)>;
        using SelectedOperation = std::function<void(SelectedComponent*)>;

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


        virtual void init() override;
        virtual void update(const float dt) override;
        virtual void imguiEditor() override;

    private:
        const SelectedOperation mResetOperation; // Called when an object is unselected
        const SelectableOperation mSelectOperation; // Called when an object is freshly selected
        const SelectedOperation mEditorOperation; // imgui
    };

}
