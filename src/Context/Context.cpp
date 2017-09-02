#include "Context.hpp"
#include "Worlds/World.hpp"
#include "Worlds/HelloWorld.hpp"

#include <stdlib.h>  // atoi
#include <string.h>  // strcmp
#include <iostream>  // cout

int Context::init(World *world) {
   int ret = display.init();
   if (ret) {
      return ret; 
   }

   // TODO: world = processargs.world
   world = new HelloWorld();
   display.setTitle(world->name.c_str());

   return 0;
}

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

void Context::printError(int error) {
   std::cout << "Error " << error << std::endl;
   switch(error) {
      case 1:
         std::cout << "Invalid arguments" << std::endl;
         break;
      case 2:
         std::cout << "Error initializing display" << std::endl;
      default:
         break;
   }
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