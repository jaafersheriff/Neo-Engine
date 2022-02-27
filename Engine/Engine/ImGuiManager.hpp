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
	private:
		static bool mIsEnabled;
	};
}