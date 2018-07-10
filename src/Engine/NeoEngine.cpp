#include "NeoEngine.hpp"

#include <time.h>
#include <iostream>

namespace neo {

    std::string NeoEngine::RESOURCE_DIR = "../res/";
    std::string NeoEngine::APP_NAME = "Neo Engine";

    /* FPS */
    int NeoEngine::FPS = 0;
    int NeoEngine::nFrames = 0;
    double NeoEngine::timeStep = 0.0;
    double NeoEngine::lastFPSTime = 0.0;
    double NeoEngine::lastFrameTime = 0.0;
    double NeoEngine::runTime = 0.0;

    void NeoEngine::init(const std::string &title, const int width, const int height) {
        srand((unsigned int)(time(0)));

        APP_NAME = title;

        /* Init window*/
        if (Window::initGLFW(APP_NAME)) {
            std::cerr << "Failed initializing Window" << std::endl;
        }
        Window::setSize(glm::ivec2(width, height));
    }

    void NeoEngine::run() {

        while (!Window::shouldClose()) {
            /* Update delta time and FPS */
            updateFPS();

            /* Update display, mouse, keyboard */
            Window::update();
        }
    }

    void NeoEngine::shutDown() {
        Window::shutDown();
    }

    void NeoEngine::updateFPS() {
        runTime = glfwGetTime();
        timeStep = runTime - lastFrameTime;
        lastFrameTime = runTime;
        nFrames++;
        if (runTime - lastFPSTime >= 1.0) {
            FPS = nFrames;
            nFrames = 0;
            lastFPSTime = runTime;
        }
    }
}