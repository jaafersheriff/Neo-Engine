/////////////////////////////////////////////////////////////////////////////////////////
//                           Abstract parent World class                               //
//                                                                                     //
// How to create a derived World:                                                      //
//    World-specific parameters: data structure to be rendered                         //
//    Constructor(): include name of world                                             //
//    Init(): Create objects and initialize rendering data struct                      //
//    prepareRenderer(): activate feature renderers and pass in proper data struct     //
//    update(): Update features, call takeInput()                                      //
//    takeInput(): update camera and features based on hardware input                  //
//    cleanUp(): Clean up features                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "Renderer/MasterRenderer.hpp"
#include "Context/Context.hpp"
#include "Cameras/Camera.hpp"
#include "Lights/Light.hpp"
#include "Toolbox/Loader.hpp"

#include <string>
#include <vector>

class World {
   public:
      World(const std::string n) : name(n) { }
      std::string name;
      Camera camera;
      std::vector<Light> lights;

      // Abstract functions
      virtual void init(Loader *) = 0;
      virtual void prepareRenderer(MasterRenderer &) = 0;
      virtual void update(Context &) = 0;
      virtual void cleanUp() = 0;

   private:
      virtual void takeInput(Mouse &, Keyboard &) = 0;
};

#endif