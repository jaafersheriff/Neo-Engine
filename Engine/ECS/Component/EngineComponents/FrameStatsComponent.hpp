#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui/imgui.h>

namespace neo {

    struct FrameStatsComponent : public Component {
        float mRunTime;
        float mDT;

        virtual std::string getName() override {
            return "FrameStatsComponent";
        }

        virtual void imGuiEditor() override {
            ImGui::TextWrapped("Run Time:   %0.3f", mRunTime);
            ImGui::TextWrapped("Frame Time: %0.3f", mDT);

        }
    };
}