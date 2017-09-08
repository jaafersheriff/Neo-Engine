#pragma once
#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include "Display.hpp"
#include "Hardware/Mouse.hpp"
#include "Hardware/Keyboard.hpp"

class World;
class Context {
   public:
      Display display;
      Mouse mouse;
      Keyboard keyboard;

      // TODO : world type enum 
      // TODO : Set world + default
      // TODO : arg <optional value> booleans or something

      int processArgs(int, char**);
      void init();
      World* createWorld(); 
      void update();
      bool shouldClose();
      void cleanUp();
};

#endif 