#pragma once

#include "Renderer/Renderer.hpp"
#include "Loader/Library.hpp"
#include "Util/Util.hpp"

#include "ECS/ECS.hpp"

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

    struct EngineConfig {
        std::string APP_NAME = "Neo Engine";
        std::string APP_RES = "";
        int width = 1920;
        int height = 1080;
        bool attachEditor = true;
    };

    class Engine {

        /* Base Engine */
        public:
            static EngineConfig mConfig;
            static void init(EngineConfig);
            static void run();
            static void shutDown();

        public:
            /* ImGui */
            static bool mImGuiEnabled;
            static void toggleImGui() { mImGuiEnabled = !mImGuiEnabled; }
            static void addImGuiFunc(std::string name, std::function<void()> func) { mImGuiFuncs.insert({ name, func}); }

        private:
            static ECS mECS;

            /* ImGui */
            static std::unordered_map<std::string, std::function<void()>> mImGuiFuncs;
            static void _runImGui(const util::FrameCounter&);

            /* Hardware */
            static WindowSurface mWindow;
            static Keyboard mKeyboard;
            static Mouse mMouse;
    };

}