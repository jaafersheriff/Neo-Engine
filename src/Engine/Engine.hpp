#pragma once

#include "Window/Window.hpp"

namespace neo {

    class Engine {
    
        public:
            static void init(const std::string &, const int, const int);
            static void run();
    };
}