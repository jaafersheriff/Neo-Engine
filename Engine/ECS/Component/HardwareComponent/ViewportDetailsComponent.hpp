#pragma once

#include "ECS/Component/Component.hpp"

#include "Hardware/WindowDetails.hpp"

#include <imgui.h>

namespace neo {

	START_COMPONENT(ViewportDetailsComponent);
		ViewportDetailsComponent(glm::uvec2 size, glm::uvec2 pos) 
			: mSize(size)
			, mPos(pos)
		{}

		virtual void imGuiEditor() override {
			ImGui::Text("Viewport Size: [%d, %d]", mSize.x, mSize.y);
			ImGui::Text("Viewport Pos:  [%d, %d]", mPos.x, mPos.y);
		}

		glm::uvec2 mSize;
		glm::uvec2 mPos;
	END_COMPONENT();
}