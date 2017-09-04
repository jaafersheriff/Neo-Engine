// Abstract parent World class
#pragma once
#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "Renderer/MasterRenderer.hpp"
#include "Context/Context.hpp"

#include <iostream>
#include <string> // name

class World {
   public:
      World(std::string n) : name(n) { }
      std::string name;
      // TODO: Camera camera;
      // TODO: std::vector<Light> lights;

      // Abstract functions
      virtual void init(/* TODO: Loader loader*/) = 0;
      virtual void prepareRenderer(MasterRenderer &) = 0;
      virtual void update(Context &) = 0;
      virtual void cleanUp() = 0;

   private:
      virtual void takeInput(Mouse &, Keyboard &) = 0;
};

#endif