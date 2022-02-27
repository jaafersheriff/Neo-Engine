#include "ImGuiManager.hpp"

#include "Renderer/Renderer.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "microprofile.h"

namespace neo {

    bool ImGuiManager::mIsEnabled = true;

    void ImGuiManager::init(GLFWwindow* window) {
        /* Init ImGui */
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, false);
        ImGui_ImplOpenGL3_Init(Renderer::NEO_GLSL_VERSION.c_str());
        ImGui::GetStyle().ScaleAllSizes(2.f);
    }

    void ImGuiManager::update() {
        if (mIsEnabled) {
            MICROPROFILE_ENTERI("Window", "ImGui::NewFrame", MP_AUTO);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            MICROPROFILE_LEAVE();
        }
    }

    void ImGuiManager::run() {
        MICROPROFILE_ENTERI("Engine", "_runImGui", MP_AUTO);
        ImGui::GetIO().FontGlobalScale = 2.0f;

        // TODO

        MICROPROFILE_LEAVE();
    }

    void ImGuiManager::render() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiManager::destroy() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}