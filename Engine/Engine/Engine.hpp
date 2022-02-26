#pragma once

#include "Renderer/Renderer.hpp"
#include "Loader/Library.hpp"
#include "Util/Util.hpp"

#include "ECS/ECS.hpp"
#include "DemoInfra/DemoWrangler.h"

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
            Engine() = default;
            ~Engine() = default;
            Engine(const Engine &) = delete;
            Engine & operator=(const Engine &) = delete;
            Engine(Engine &&) = delete;
            Engine & operator=(Engine &&) = delete;

            static void init();
            static void run(DemoWrangler&);
            static void shutDown();

            /* ImGui */
            static bool mImGuiEnabled;
            static void toggleImGui() { mImGuiEnabled = !mImGuiEnabled; }
            using ImGuiFunc = std::function<void(ECS& ecs)>;
            static void addImGuiFunc(std::string name, ImGuiFunc func) { mImGuiFuncs.insert({ name, func}); }

        private:
            static ECS mECS;
            static void _createPrefabs();
			static void _swapDemo(DemoWrangler&);

            /* ImGui */
            static std::unordered_map<std::string, ImGuiFunc> mImGuiFuncs;
            static void _runImGui(DemoWrangler&, const util::FrameCounter&);

            /* Hardware */
            static WindowSurface mWindow;
            static Keyboard mKeyboard;
            static Mouse mMouse;
    };

}