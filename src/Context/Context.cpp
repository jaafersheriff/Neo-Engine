#include "Context.hpp"
#include "World/World.hpp"
#include "TriangleWorld/TriangleWorld.hpp"

#include <stdlib.h>  // atoi
#include <string.h>  // strcmp
#include <iostream>  // cout

int Context::processArgs(int argc, char **argv) {
   for (int i = 0; i < argc; i++) {
      char *arg = argv[i];
      if (strcmp(arg, "-s") == 0) {
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

World* Context::createWorld() {
   // TODO: create world based on enum
   World *world = new TriangleWorld();
   display.setTitle(world->name.c_str());
   return world;
}

void Context::update() { 
   display.update();
   mouse.update(display.window);
}

bool Context::shouldClose() {
   return display.shouldClose();
}

void Context::cleanUp() {
   display.cleanUp();
}