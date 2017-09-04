#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

#include <iostream>
#include "Context/Context.hpp"
#include "World/World.hpp"
#include "Renderer/MasterRenderer.hpp"

int main(int argc, char **argv) {
   World *world;
   Context context;
   MasterRenderer mr;

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
   
   // Create world
   world = context.createWorld();

   // Prep MR for rendering of a specific world class
   world->init();
   world->prepareRenderer(mr);

   // Main loop
   while(!context.shouldClose()) {
      context.update();
      world->update(context);
      
      mr.render();

      // OpenGL things
      glfwSwapBuffers(context.display.window);
      glfwPollEvents();
   }

   context.cleanUp();
   world->cleanUp();
   delete world;

	return 0;
}


