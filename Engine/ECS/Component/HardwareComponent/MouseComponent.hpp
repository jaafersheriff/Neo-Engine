#pragma once

#include "ECS/Component/Component.hpp"
#include "Hardware/Mouse.hpp"

#include <imgui.h>

namespace neo {

	START_COMPONENT(MouseComponent);
		MouseComponent(Mouse engineMouse)
			: mFrameMouse(engineMouse)
		{}

		virtual void imGuiEditor() override {
			ImGui::Text("Position: [%0.2f, %0.2f]", mFrameMouse.getPos().x, mFrameMouse.getPos().y);
			ImGui::Text("Speed:	[%0.2f, %0.2f, %0.2f]", mFrameMouse.getSpeed().x, mFrameMouse.getSpeed().y, mFrameMouse.getScrollSpeed());
		}

		Mouse mFrameMouse;
	END_COMPONENT();
}