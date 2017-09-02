#include "Context.hpp"

#include <stdlib.h>  // atoi
#include <string.h>  // strcmp
#include <iostream>  // cout

int Context::init(/* TODO : World *world */) {
   // TODO : init world 
   // TODO : catch any errors along the way

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
   // TODO : set world name
   // TODO : catch errors along the way 

   return 0;
}

void Context::printError(int error) {
   switch(error) {
      case 1:
         std::cout << "Invalid arguments" << std::endl;
         break;
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