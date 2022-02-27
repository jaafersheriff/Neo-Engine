#include "ImGuiManager.hpp"

#include "Renderer/Renderer.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "microprofile.h"

#include "GLFW/glfw3.h"

namespace neo {

    bool ImGuiManager::mIsEnabled = true;

    void ImGuiManager::init(GLFWwindow* window) {
        /* Init ImGui */
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
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

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        static bool open = true;
        ImGui::Begin("###DockSpace", &open, window_flags);
        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        NEO_ASSERT(io.ConfigFlags & ImGuiConfigFlags_DockingEnable, "");
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        ImGui::GetIO().FontGlobalScale = 2.0f;

        ImGui::Begin("Viewport");
        ImVec2 size;
        size.x = ImGui::GetWindowWidth();
        size.y = ImGui::GetWindowHeight();
        if (size.x != Renderer::mDefaultFBO->mTextures[0]->mWidth || size.y != Renderer::mDefaultFBO->mTextures[0]->mHeight) {
            Messenger::sendMessage<WindowFrameSizeMessage>(nullptr, glm::uvec2(size.x, size.y));
        }
#pragma warning(push)
#pragma warning(disable: 4312)
        ImGui::Image(reinterpret_cast<ImTextureID>(Renderer::mDefaultFBO->mTextures[0]->mTextureID), size, ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
        ImGui::End();
        // TODO

        ImGui::End();
        MICROPROFILE_LEAVE();
    }

    void ImGuiManager::render() {
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiManager::destroy() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}