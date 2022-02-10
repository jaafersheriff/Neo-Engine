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
        SelectingSystem(
            std::string name = "Selecting System",
            std::function<void(SelectableComponent*)> resetOperation = [](SelectableComponent*) {},
            std::function<void(SelectedComponent*, const MouseRayComponent*, float)> selectOperation = [](SelectedComponent*, const MouseRayComponent*, float) {},
            std::function<void(std::vector<SelectedComponent*>&)> editorOperation = [](std::vector<SelectedComponent*>&) {}) :

            System(name),
            mResetOperation(resetOperation),
            mSelectOperation(selectOperation),
            mEditorOperation(editorOperation)
        {}


        virtual void init() override;
        virtual void update(const float dt) override;
        virtual void imguiEditor() override;

    private:
        const std::function<void(SelectableComponent*)> mResetOperation;
        const std::function<void(SelectedComponent*, const MouseRayComponent *mouseRay, float delta)> mSelectOperation;
        const std::function<void(std::vector<SelectedComponent*>&)> mEditorOperation;
    };

}
