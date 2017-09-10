#pragma once
#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "Renderer/MasterRenderer.hpp"
#include "Context/Context.hpp"
#include "Cameras/Camera.hpp"

#include <iostream>
#include <string> // name

class World {
   public:
      World(const std::string n) : name(n) { }
      std::string name;
      Camera camera;
      // TODO: std::vector<Light> lights;

      // Abstract functions
      virtual void init(/* TODO: Loader loader*/) = 0;
      virtual void prepareRenderer(const MasterRenderer &) = 0;
      virtual void update(const Context &) = 0;
      virtual void cleanUp() = 0;

   private:
      virtual void takeInput(const Mouse &, const Keyboard &) = 0;
};

#endif