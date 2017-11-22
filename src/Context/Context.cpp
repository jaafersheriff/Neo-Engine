#include "Context.hpp"
#include "World/World.hpp"
#include "World/DevWorld.hpp"

#include <string.h>  /* strcmp, strlen */
#include <iostream>  /* cout, stoi */

void Context::printUsage() {
    std::cerr << "Invalid args" << std::endl;
    std::cerr << "Usage: ./Neo.exe" << std::endl;

    std::cerr << "\t-s <window width> <window height>" << std::endl;

    std::cerr << "\t-w <world_name>" << std::endl;
    std::cerr << "\t\tDEV_WORLD" << std::endl;
    // TODO : add more worlds

}

int Context::processArgs(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {

        /* Process window size */
        if (!strcmp(argv[i], "-s")) {
            /* Catch invalid number of args */
            if (i + 2 >= argc) {
                printUsage();
                return 1;
            }
            /* Load size 
             * std::stoi will catch invalid arguments */
            // TODO : use strtol and catch invalid args myself so I can use printUsage()
            display.width = std::stoi(argv[++i], nullptr);
            display.height = std::stoi(argv[++i], nullptr); 

            if (display.width <= 0 || display.height <= 0) {
                printUsage();
                return 1;
            }
            std::cout << "[" << display.width << ", " << display.height << "]" << std::endl;
        }
        /* Store chosen world */
        if (!strcmp(argv[i], "-w")) {
            /* Catch invalid number of args */
            if (++i >= argc) {
                printUsage();
                return 1;
            }

            /* Dev world */
            if(!strcmp(argv[i], "DEV_WORLD")) {
                selectedWorld = DEV_WORLD_TYPE;
            }
            /* Invalid world chosen */
            else {
                printUsage();
                return 1;
            }
        }
    }

    return 0;
}

void Context::init() {
    /* Init members */
    mouse.window = keyboard.window = display.window;

    /* Init local */
    lastTime = runningTime = glfwGetTime();
    nbFrames = 0.0;
}

World* Context::createWorld() {
    /* Create world */
    World *world;
    switch(selectedWorld) {
        case(DEV_WORLD_TYPE):
            world = new DevWorld;
            break;
        /* Error state -- we should never get here */
        default:
            return nullptr;
            break;
    }

    /* Set window name to application name */
    display.setTitle(world->name.c_str());

    return world;
}

void Context::update() {
    /* Update members */
    display.update();
    mouse.update();

    /* Update time */
    runningTime = glfwGetTime();

    /* Update FPS */
    nbFrames++;
    if (runningTime - lastTime >= 1.0) {
        fps = double(nbFrames);
        nbFrames = 0.0;
        lastTime = runningTime;
    }
}

bool Context::shouldClose() {
    return display.shouldClose();
}

void Context::cleanUp() {
    display.cleanUp();
}
