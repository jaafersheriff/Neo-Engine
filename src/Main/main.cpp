#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

#include <iostream>
#include "Context/Context.hpp"
#include "Worlds/World.hpp"

int main(int argc, char **argv) {

   int ret;
   Context context;
   World *world;

   // Process args
   ret = context.processArgs(argc, argv);
   if (ret) {
      context.printError(ret);
      return ret;
   }

   // Init context
   ret = context.init(world);
   if (ret) {
      context.printError(ret);
      return ret;
   }
   
   // TODO: world.preprenderer

   // Main loop
   while(!context.shouldClose()) {
      context.update();
      world->update(context);
      // TODO: mr->render

      // OpenGL things
      glfwSwapBuffers(context.display.window);
      glfwPollEvents();
   }

   context.cleanUp();
   world->cleanUp();
   delete world;

	return 0;
}


