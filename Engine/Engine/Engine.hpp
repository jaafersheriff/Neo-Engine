#pragma once

#include "Renderer/Renderer.hpp"
#include "Loader/Library.hpp"
#include "Util/Util.hpp"

#include "ECS/ECS.hpp"
#include "DemoInfra/IDemo.hpp"

#include "ext/imgui/imgui.h"
#include "microprofile.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>
#include <thread>

#include "ECS/GameObject.hpp"

namespace neo {
    namespace util {
        struct FrameCounter;
    }
    class ComponentTuple;
    class Window;
    class Keyboard;
    class Mouse;

    class Engine {

        /* Base Engine */
        public:
            static ECS& init();
            static void run(std::vector<neo::IDemo*>& demos, uint32_t currDemo);
            static void shutDown();

        public:
            /* ImGui */
            static bool mImGuiEnabled;
            static void toggleImGui() { mImGuiEnabled = !mImGuiEnabled; }
            using ImGuiFunc = std::function<void(ECS& ecs)>;
            static void addImGuiFunc(std::string name, ImGuiFunc func) { mImGuiFuncs.insert({ name, func}); }

        private:
            static ECS mECS;

            /* ImGui */
            static std::unordered_map<std::string, ImGuiFunc> mImGuiFuncs;
            static void _runImGui(std::vector<neo::IDemo*>& demos, uint32_t currDemo, const util::FrameCounter&);

            /* Hardware */
            static WindowSurface mWindow;
            static Keyboard mKeyboard;
            static Mouse mMouse;
    };

}