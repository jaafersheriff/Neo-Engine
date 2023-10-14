#pragma once

#include "Renderer/Renderer.hpp"
#include "Loader/Library.hpp"
#include "Util/Util.hpp"

#include "ECS/ECS.hpp"
#include "DemoInfra/DemoWrangler.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <optional>
#include <thread>

namespace neo {
    namespace util {
        struct FrameCounter;
    }
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

        private:
            static ECS mECS;
            static void _createPrefabs();
			static void _swapDemo(DemoWrangler&);

            /* Hardware */
            static WindowSurface mWindow;
            static Keyboard mKeyboard;
            static Mouse mMouse;
    };

}