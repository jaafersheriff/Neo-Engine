#pragma once

#include "Systems/System.hpp"

namespace neo {

    class SelectingSystem : public System {

    public:
        SelectingSystem(
            int maxMarches = 100,
            float maxDist = 100.f,
            std::function<bool(SelectedComponent*)> removeDecider = [](SelectedComponent*) { return true; },
            std::function<void(SelectableComponent*)> resetOperation = [](SelectableComponent*) {},
            std::function<void(SelectedComponent*, glm::vec3)> selectOperation = [](SelectedComponent*, glm::vec3) {},
            std::function<void(std::vector<SelectedComponent*>&)> editorOperation = [](std::vector<SelectedComponent*>&) {}) :

            System("Selecting System"),
            mMaxMarches(maxMarches),
            mMaxDist(maxDist),
            mRemoveDecider(removeDecider),
            mResetOperation(resetOperation),
            mSelectOperation(selectOperation),
            mEditorOperation(editorOperation)
        {}


        virtual void init() override;
        virtual void update(const float dt) override;
        virtual void imguiEditor() override;

    private:
        const int mMaxMarches;
        const float mMaxDist;
        const std::function<bool(SelectedComponent*)> mRemoveDecider;
        const std::function<void(SelectableComponent*)> mResetOperation;
        const std::function<void(SelectedComponent*, glm::vec3)> mSelectOperation;
        const std::function<void(std::vector<SelectedComponent*>&)> mEditorOperation;
    };

}
