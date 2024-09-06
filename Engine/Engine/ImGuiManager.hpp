#pragma once

#include "Util/Log/ImGuiConsole.hpp"
#include "Util/Log/Log.hpp"

#include "ResourceManager/MeshManager.hpp"

#include <ext/imgui_incl.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

struct GLFWwindow;

namespace neo {
	class ECS;
	class ResourceManagers;

	class ImGuiManager {
	public:
		struct Viewport {
			bool mIsFocused = false;
			bool mIsHovered = false;
			glm::uvec2 mOffset = { 0,0 };
			glm::uvec2 mSize = { 0,0 };
		};

		ImGuiManager() = default;
		~ImGuiManager() = default;
		ImGuiManager(const ImGuiManager&) = delete;
		ImGuiManager & operator=(const ImGuiManager&) = delete;

		void init(GLFWwindow* window, float dpiScale);
		void update();
		void render();
		void reset();
		void destroy();

		void reload(ResourceManagers& resourceManagers);
		void resolveDrawData(ECS& ecs, ResourceManagers& resourceManagers);

		void begin();
		void end();

		void log(const char* log, util::LogSeverity severity);
		void imGuiEditor();

		void updateMouse(GLFWwindow* window, int button, int action, int mods);
		void updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
		void updateCharacter(GLFWwindow* window, unsigned int c);
		void updateScroll(GLFWwindow* window, double dx, double dy);

		void toggleImGui();
		bool isEnabled() { return mIsEnabled; }

		void updateViewport();
		bool isViewportFocused();
		bool isViewportHovered();
		glm::uvec2 getViewportOffset();
		glm::uvec2 getViewportSize();
	private:
		bool mIsEnabled = true;
		Viewport mViewport;
		ImGuiConsole mConsole;
		
		std::array<MeshHandle, 16> mImGuiMeshes;
	};
}