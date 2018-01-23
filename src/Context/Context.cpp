#include "Context.hpp"
#include "World/World.hpp"
#include "World/CloudWorld/CloudWorld.hpp"
#include "World/SkyWorld/SkyWorld.hpp"

#include <string.h>  /* strcmp, strlen  */
#include <iostream>  /* cout, stoi      */

void Context::printUsage() {
    std::cout << "Usage: Neo -w <world_name>" << std::endl;

    std::cout << "    -w <world_name>";
    std::cout << "\t\t\tSet world/application type" << std::endl;

    std::cout << "    -r <resources dir>";
    std::cout << "\tSet the resource directory" << std::endl;

    std::cout << "    -s <window width> <window height>";
    std::cout << "\tSet window size" << std::endl;
}

int Context::processArgs(int argc, char **argv) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    for (int i = 0; i < argc; i++) {

        /* Help */
        if (!strcmp(argv[i], "-h")) {
            printUsage();
            return 1;
        }
        /* Set verbose */
        if (!strcmp(argv[i], "-v")) {
            verbose = true;
        }
        /* Set resources dir */
        if (!strcmp(argv[i], "-r")) {
            if (i + 1 >= argc) {
                printUsage();
                return 1;
            }
            RESOURCE_DIR = argv[i+1];
        }
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
            if(!strcmp(argv[i], "CLOUD_WORLD")) {
                selectedWorld = CLOUD_WORLD;
            }
            /* Sky world */
            else if(!strcmp(argv[i], "SKY_WORLD")) {
                selectedWorld = SKY_WORLD;
            }
            /* Invalid world chosen */
            else {
                printUsage();
                return 1;
            }
        }
    }

    if (selectedWorld == ERROR) {
        printUsage();
        return 1;
    }

    return 0;
}

void Context::init() {
    /* Init members */
    mouse.window = keyboard.window = display.window;

    /* Init local */
    lastFrameTime = runTime = (float) glfwGetTime();
    timeStep = lastFpsTime = 0.f;
    nFrames = 0;
}

World* Context::createWorld() {
    /* Create world */
    World *world;
    switch(selectedWorld) {
        case(CLOUD_WORLD):
            world = new CloudWorld;
            break;
        case(SKY_WORLD):
            world = new SkyWorld;
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
    runTime = (float) glfwGetTime();
    timeStep = (runTime - lastFrameTime);
    lastFrameTime = runTime;

    /* Update FPS */
    nFrames++;
    if (runTime - lastFpsTime >= 1.0) {
        fps = double(nFrames);
        nFrames = 0;
        lastFpsTime = runTime;
        if (verbose) {
            std::cout << "FPS: " << fps << std::endl;
        }
    }
}

bool Context::shouldClose() {
    return display.shouldClose();
}

void Context::cleanUp() {
    display.cleanUp();
}
