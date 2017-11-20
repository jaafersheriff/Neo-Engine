#include "Context.hpp"
#include "World/World.hpp"
#include "World/EntityWorld.hpp"

#include <stdlib.h>  /* atoi */
#include <string.h>  /* strcmp */
#include <iostream>  /* cout */

int Context::processArgs(int argc, char **argv) {

    for (int i = 0; i < argc; i++) {
        char *arg = argv[i];
        /* Process window size */
        if (!strcmp(arg, "-s")) {
            if (i > argc - 2) {
                return 1;
            }
            display.width = atoi(argv[++i]);
            display.height = atoi(argv[++i]);
            std::cout << "[" << display.width << ", " << display.height << "]" << std::endl;
        }
    }
    // TODO : set world emum
    // TODO : catch errors along the way 

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
    // TODO: create world based on enum
    World *world = new EntityWorld;

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
