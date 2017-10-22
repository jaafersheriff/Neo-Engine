#include "Context.hpp"
#include "World/World.hpp"
#include "World/EntityWorld.hpp"

#include <stdlib.h>  // atoi
#include <string.h>  // strcmp
#include <iostream>  // cout
#include <time.h>    // clock

int Context::processArgs(int argc, char **argv) {
   for (int i = 0; i < argc; i++) {
      char *arg = argv[i];
      
      // Window size 
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
   mouse.window = keyboard.window = display.window;
}

World* Context::createWorld() {
   // TODO: create world based on enum
   World *world = new EntityWorld;
   display.setTitle(world->name.c_str());
   return world;
}

void Context::update() { 
   display.update();
   mouse.update();
   runningTime = clock()/(CLOCKS_PER_SEC/1000.f);
}

bool Context::shouldClose() {
   return display.shouldClose();
}

void Context::cleanUp() {
   display.cleanUp();
}