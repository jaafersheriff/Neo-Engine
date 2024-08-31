#pragma once

#include "ECS/Component/Component.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	START_COMPONENT(FrameStatsComponent);
		FrameStatsComponent(float rt, float dt)
			: mRunTime(rt)
			, mDT(dt)
		{}

		virtual void imGuiEditor() override {
			ImGui::TextWrapped("Run Time:   %0.3f", mRunTime);
			ImGui::TextWrapped("Frame Time: %0.3f", mDT);

		}

		float mRunTime;
		float mDT;
	END_COMPONENT();
}
