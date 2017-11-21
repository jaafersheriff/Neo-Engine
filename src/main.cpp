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
    
    Context context;    // GLFWwindow, Mouse, Keyboard
    Loader loader;      // Load .obj models and .png textures
    MasterRenderer mr;  // Renderer
    World *world;       // Application - collection of features

    // Process args
    if (context.processArgs(argc, argv)) {
        std::cerr << "Invalid args " << std::endl;
        std::cerr << "Usage: ./Neo.exe" << std::endl;
        std::cerr <<"\t-s <window width> <window height>" << std::endl;
    }

    // Init display
    if (context.display.init()) {
        std::cerr << "Failed to init display" << std::endl;
    }
    context.init();

    // Create world
    world = context.createWorld();

    // Prep MR for rendering of a specific world class
    mr.init();
    world->init(loader);
    world->prepareRenderer(mr);

    // Main loop
    while(!context.shouldClose()) {
        context.update();
        world->update(context);
        
        mr.render(context.display, world);

        // OpenGL things
        glfwSwapBuffers(context.display.window);
        glfwPollEvents();
    }

    context.cleanUp();
    world->cleanUp();
    mr.cleanUp();
    delete world;

    return 0;
}


