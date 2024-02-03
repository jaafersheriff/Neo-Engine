#include "ImGuiManager.hpp"

#include "Messaging/Messenger.hpp"

#include "Engine/Engine.hpp"
#include "Renderer/Renderer.hpp"
#include "Hardware/WindowSurface.hpp"
#include "Hardware/Mouse.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"
#include "Util/Log/Log.hpp"
#include "Util/ServiceLocator.hpp"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace neo {

    void ImGuiManager::init(GLFWwindow* window) {
        /* Init ImGui */
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // Tracy does its own font scaling. Because of course it does.
#ifdef NO_LOCAL_TRACY
        io.FontGlobalScale = 2.f;
#endif
        ImGui_ImplGlfw_InitForOpenGL(window, false);
        ImGui_ImplOpenGL3_Init(ServiceLocator<Renderer>::ref().mDetails.mGLSLVersion.c_str());

        ImGuiStyle* style = &ImGui::GetStyle();
        style->ScaleAllSizes(io.FontGlobalScale);
        ImVec4* colors = style->Colors;
#define HEXTOIM(x) x >= 0x1000000 ? ImVec4( ((x >> 24) & 0xFF) /255.f, ((x >> 16) & 0xFF)/255.f, ((x >> 8) & 0xFF)/255.f, ((x) & 0xFF)/255.f): ImVec4( ((x >> 16) & 0xFF) /255.f, ((x >> 8) & 0xFF)/255.f, ((x) & 0xFF)/255.f, 1.f) 
        auto texColglm = util::sLogSeverityData.at(util::LogSeverity::Info).second;
        auto texColimv = ImVec4(texColglm.x, texColglm.y, texColglm.z, 1.f);
        colors[ImGuiCol_Text] = texColimv;
        auto disabledTexColglm = util::sLogSeverityData.at(util::LogSeverity::Verbose).second;
        auto disabledTexColimv = ImVec4(disabledTexColglm.x, disabledTexColglm.y, disabledTexColglm.z, 1.f);
        colors[ImGuiCol_TextDisabled] = disabledTexColimv;
        colors[ImGuiCol_WindowBg] = HEXTOIM(0x070808);
        colors[ImGuiCol_Border] = HEXTOIM(0x434343);
        colors[ImGuiCol_FrameBg] = HEXTOIM(0x101B15);
        colors[ImGuiCol_FrameBgHovered] = HEXTOIM(0x393D3A);
        colors[ImGuiCol_FrameBgActive] = HEXTOIM(0x454f47);
        colors[ImGuiCol_TitleBg] = HEXTOIM(0x151917);
        colors[ImGuiCol_TitleBgActive] = HEXTOIM(0x1f2824);
        colors[ImGuiCol_DragDropTarget] = texColimv;
        colors[ImGuiCol_DockingPreview] = HEXTOIM(0x373f38);
        colors[ImGuiCol_TabHovered] = HEXTOIM(0x373f38);
        colors[ImGuiCol_TabActive] = HEXTOIM(0x394738);
        colors[ImGuiCol_Tab] = HEXTOIM(0x1F2622);
        colors[ImGuiCol_TabUnfocused] = HEXTOIM(0x1F2622);
        colors[ImGuiCol_TabUnfocusedActive] = HEXTOIM(0x19201C);
        colors[ImGuiCol_CheckMark] = texColimv;
        colors[ImGuiCol_SliderGrab] = disabledTexColimv;
        colors[ImGuiCol_SliderGrabActive] = texColimv;
        colors[ImGuiCol_Button] = HEXTOIM(0x1f2824);
        colors[ImGuiCol_ButtonHovered] = HEXTOIM(0x393D3A);
        colors[ImGuiCol_ButtonActive] = HEXTOIM(0x454f47);
        colors[ImGuiCol_Header] = HEXTOIM(0x171c19);
        colors[ImGuiCol_HeaderHovered] = HEXTOIM(0x393D3A);
        colors[ImGuiCol_HeaderActive] = HEXTOIM(0x454f47);
        colors[ImGuiCol_ResizeGrip] = HEXTOIM(0x373f38);
        colors[ImGuiCol_ResizeGripHovered] = disabledTexColimv;
        colors[ImGuiCol_ResizeGripActive] = texColimv;
        colors[ImGuiCol_SeparatorHovered] = disabledTexColimv;
        colors[ImGuiCol_SeparatorActive] = texColimv;
        // ImPlot::GetStyle().Colors[ImPlotCol_Line] = texColimv;

        style->FrameRounding = 0.0f;
        style->WindowBorderSize = 0.f;
        style->FrameBorderSize = 0.f;
        style->ChildBorderSize = 0.f;
        style->WindowPadding = { 4.f, 4.f };
        style->ItemSpacing = { 8.f, 4.f };
        style->GrabMinSize = 10.f;

        style->ChildRounding = 0.0f;
        style->PopupRounding = 1.0f;
        style->ScrollbarRounding = 0.0f;
        style->ScrollbarSize = 13.0f;
        style->TabBorderSize = 0.0f;
        style->TabRounding = 0.0f;
        style->WindowRounding = 0.0f;
        style->GrabRounding = 0.f;
        style->WindowMenuButtonPosition = ImGuiDir_None;
        style->ColorButtonPosition = ImGuiDir_Left;
        style->AntiAliasedFill = false;
        style->AntiAliasedFill = false;

    }

    void ImGuiManager::update() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");
        TRACY_ZONE();

        {
            TRACY_ZONEN("ImGui_ImplOpenGL3_NewFrame");
            ImGui_ImplOpenGL3_NewFrame();
        }
        {
            TRACY_ZONEN("ImGui_ImplGlfw_NewFrame");
            ImGui_ImplGlfw_NewFrame();
        }
        {
            TRACY_ZONEN("ImGui::NewFrame");
            ImGui::NewFrame();
        }

        if (isViewportHovered()) {
            TRACY_ZONEN("MouseMoveMessage");
            auto [mx, my] = ImGui::GetMousePos();
            mx -= mViewport.mOffset.x;
            my -= mViewport.mOffset.y;
            my = mViewport.mSize.y - my;
            if (mx >= 0 && mx <= mViewport.mSize.x && my >= 0 && my <= mViewport.mSize.y) {
                Messenger::sendMessage<Mouse::MouseMoveMessage>(static_cast<double>(mx), static_cast<double>(my));
            }
        }
    }

    void ImGuiManager::updateViewport() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");
        TRACY_ZONE();

        // TODO -- this can all grab the necessary details directly using the Viewport's ImGuiID 
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
                Messenger::sendMessage<FrameSizeMessage>(mViewport.mSize);
            }
        }
    }

    void ImGuiManager::updateMouse(GLFWwindow* window, int button, int action, int mods) {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }

    void ImGuiManager::updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }

    void ImGuiManager::updateScroll(GLFWwindow* window, double dx, double dy) {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
    }

    void ImGuiManager::updateCharacter(GLFWwindow* window, unsigned int c) {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        ImGui_ImplGlfw_CharCallback(window, c);
    }

    void ImGuiManager::begin() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");
        TRACY_ZONEN("ImGuiManager::begin");

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_HorizontalScrollbar;

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
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoUndocking);
    }

    void ImGuiManager::end() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");
        TRACY_ZONEN("ImGuiManager::end");

        ImGui::End();
    }

    void ImGuiManager::render() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        {
            TRACY_GPUN("ImGui::render");
            ImGui::Render();
        }
        {

            TRACY_GPUN("ImGui_ImplOpenGL3_RenderDrawData");
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        // Only needed with multiple viewports, which I don't do
        // 
        // ImGuiIO& io = ImGui::GetIO();
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     
        //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //     {
        //         TRACY_GPUN("ImGui::UpdatePlatformWindows");
        //         ImGui::UpdatePlatformWindows();
        //     }
        //     {

        //         TRACY_GPUN("ImGui::RenderPlatformWindowsDefault");
        //         ImGui::RenderPlatformWindowsDefault();
        //     }
        // 
        //      glfwMakeContextCurrent(backup_current_context);
        //     
        // }
    }

    void ImGuiManager::toggleImGui() {
        NEO_LOG_I("Toggle imgui");
        mIsEnabled = !mIsEnabled;
        reset();
    }

    void ImGuiManager::reset() {
        mViewport = {};
    }

    bool ImGuiManager::isViewportFocused() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        return mViewport.mIsFocused;
    }

    bool ImGuiManager::isViewportHovered() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        return mViewport.mIsHovered;
    }

    glm::uvec2 ImGuiManager::getViewportSize() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        // Viewport isn't ready on the first frame..
        if (mViewport.mSize == glm::uvec2(0, 0)) {
            return glm::uvec2(1, 1);
        }
        return mViewport.mSize;
    }

    glm::uvec2 ImGuiManager::getViewportOffset() {
        NEO_ASSERT(mIsEnabled, "ImGui is disabled");

        return mViewport.mOffset;
    }

    void ImGuiManager::destroy() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    void ImGuiManager::log(const char* log, util::LogSeverity severity) {
        NEO_UNUSED(log, severity);
		mConsole.addLog(log, severity);
    }

    void ImGuiManager::imGuiEditor() {
        mConsole.imGuiEditor();
    }
}
