#include "ImGuiManager.hpp"

#include "Engine/Engine.hpp"
#include "Renderer/Renderer.hpp"
#include "Hardware/Mouse.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "microprofile.h"

#include "GLFW/glfw3.h"

namespace neo {

    bool ImGuiManager::mIsEnabled = true;
    ImGuiManager::Viewport ImGuiManager::mViewport;

    void ImGuiManager::init(GLFWwindow* window) {
        /* Init ImGui */
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui_ImplGlfw_InitForOpenGL(window, false);
        ImGui_ImplOpenGL3_Init(Renderer::NEO_GLSL_VERSION.c_str());
        // ImGui::GetStyle().ScaleAllSizes(2.f);
    }

    void ImGuiManager::update() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        MICROPROFILE_SCOPEI("ImGuiManager", "ImGuiManager::update", MP_AUTO);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (isViewportHovered()) {
            auto [mx, my] = ImGui::GetMousePos();
            mx -= mViewport.mOffset.x;
            my -= mViewport.mOffset.y;
            my = mViewport.mSize.y - my;
            if (mx >= 0 && mx <= mViewport.mSize.x && my >= 0 && my <= mViewport.mSize.y) {
                Messenger::sendMessage<Mouse::MouseMoveMessage>(nullptr, static_cast<double>(mx), static_cast<double>(my));
            }
        }
    }

    void ImGuiManager::updateViewport() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        MICROPROFILE_SCOPEI("ImGuiManager", "ImGuiManager::updateViewport", MP_AUTO);

        mViewport.mIsFocused = ImGui::IsWindowFocused();
        mViewport.mIsHovered = ImGui::IsWindowHovered();

        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
        glm::ivec2 offset = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        glm::ivec2 size = { viewportMaxRegion.x - viewportMinRegion.x, viewportMaxRegion.y - viewportMinRegion.y };
        if (size.x != 0 && size.y != 0) {
            if (glm::uvec2(size) != mViewport.mSize || glm::uvec2(offset) != mViewport.mOffset) {
                mViewport.mOffset = glm::uvec2(offset);
                mViewport.mSize = glm::uvec2(size);
                Messenger::sendMessage<FrameSizeMessage>(nullptr, mViewport.mSize);
            }
        }
    }

    void ImGuiManager::updateMouse(GLFWwindow* window, int button, int action, int mods) {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }

    void ImGuiManager::updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }

    void ImGuiManager::updateScroll(GLFWwindow* window, double dx, double dy) {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
    }

    void ImGuiManager::updateCharacter(GLFWwindow* window, unsigned int c) {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        ImGui_ImplGlfw_CharCallback(window, c);
    }

    void ImGuiManager::begin() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        MICROPROFILE_SCOPEI("ImGuiManager", "ImGuiManager::begin", MP_AUTO);

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_HorizontalScrollbar;

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
        // ImGui::GetIO().FontGlobalScale = 2.0f;

    }

    void ImGuiManager::end() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        ImGui::End();
    }

    void ImGuiManager::render() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        {
            MICROPROFILE_SCOPEI("ImGuiManager", "ImGui::render", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("ImGui::render", MP_AUTO);
            ImGui::Render();
        }
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            auto draw = ImGui::GetDrawData();
            if (draw) {
                printf("ds %0.2f %0.2f\n", draw->DisplaySize.x, draw->DisplaySize.y);
                printf("dp %0.2f %0.2f\n", draw->DisplayPos.x, draw->DisplayPos.y);
            }
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            
            {
                MICROPROFILE_SCOPEI("ImGuiManager", "ImGui::UpdatePlatformWindows", MP_AUTO);
                MICROPROFILE_SCOPEGPUI("ImGui::UpdatePlatformWindows", MP_AUTO);
                ImGui::UpdatePlatformWindows();
            }
            {

                MICROPROFILE_SCOPEI("ImGuiManager", "ImGui::RenderPlatformWindowsDefault", MP_AUTO);
                MICROPROFILE_SCOPEGPUI("ImGui::RenderPlatformWindowsDefault", MP_AUTO);
                ImGui::RenderPlatformWindowsDefault();
            }
            {

                MICROPROFILE_SCOPEI("ImGuiManager", "ImGui_ImplOpenGL3_RenderDrawData", MP_AUTO);
                MICROPROFILE_SCOPEGPUI("ImGui_ImplOpenGL3_RenderDrawData", MP_AUTO);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
            
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiManager::toggleImGui() {
        mIsEnabled = !mIsEnabled;
        reset();
    }

    void ImGuiManager::reset() {
        mViewport = {};
    }

    bool ImGuiManager::isViewportFocused() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        return mViewport.mIsFocused;
    }

    bool ImGuiManager::isViewportHovered() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        return mViewport.mIsHovered;
    }

    glm::uvec2 ImGuiManager::getViewportSize() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        return mViewport.mSize;
    }

    glm::uvec2 ImGuiManager::getViewportOffset() {
        NEO_ASSERT(mIsEnabled, "Don't call imgui stuff if it isn't enabled");

        return mViewport.mOffset;
    }

    void ImGuiManager::destroy() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}