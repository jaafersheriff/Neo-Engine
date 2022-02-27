#pragma once

#include "imgui/imgui.h"

struct GLFWwindow;

namespace neo {
	class ImGuiManager {
	public:

		static void init(GLFWwindow* window);
		static void update();
		static void run();
		static void render();
		static void destroy();

        static void toggleImGui() { mIsEnabled = !mIsEnabled; }
		static bool isEnabled() { return mIsEnabled; }
	private:
		static bool mIsEnabled;
	};
}