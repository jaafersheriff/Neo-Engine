#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui/imgui.h>

namespace neo {

    struct FrameStatsComponent : public Component {
        FrameStatsComponent(float rt, float dt)
            : mRunTime(rt)
            , mDT(dt)
        {}

        virtual std::string getName() const override {
            return "FrameStatsComponent";
        }

        virtual void imGuiEditor() override {
            ImGui::TextWrapped("Run Time:   %0.3f", mRunTime);
            ImGui::TextWrapped("Frame Time: %0.3f", mDT);

        }

        float mRunTime;
        float mDT;
    };
}