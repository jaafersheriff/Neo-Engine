#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

#include <iostream>
#include <time.h>

#include "Context/Context.hpp"
#include "World/World.hpp"
#include "Renderer/MasterRenderer.hpp"

int main(int argc, char **argv) {
    srand(time(0));
    
    Context context;    /* GLFWwindow, Mouse, Keyboard          */
    Loader loader;      /* Load .obj models and .png textures   */
    MasterRenderer mr;  /* Renderer                             */
    World *world;       /* Application - collection of features */

    /* Process args */
    if (context.processArgs(argc, argv)) {
        return 1;
    }

    /* Init display */
    if (context.display.init()) {
        std::cerr << "Failed to init display" << std::endl;
        return 1;
    }
    context.init();
    loader.init(context);

    /* Create world */
    world = context.createWorld();

    /* Prep MR for rendering of a specific world class */
    mr.init(context);
    world->init(context, loader);
    world->prepareRenderer(&mr);
    world->prepareUniforms(&mr);

    /* Main loop */
    while(!context.shouldClose()) {
        context.update();

        world->update(context);
        mr.render(context.display, world);

        context.display.swap();
    }

    context.cleanUp();
    world->cleanUp();
    mr.cleanUp();
    delete world;

    return 0;
}


