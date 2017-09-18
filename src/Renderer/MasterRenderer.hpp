// Master Renderer class
// Contains all subrenderers
// World class activates different subrenderers
// Master Renderer only renders active subrenderers 
#pragma once
#ifndef _MASTER_RENDERER_HPP_
#define _MASTER_RENDERER_HPP_

#include "Context/Display.hpp"
#include "Renderer.hpp"

#include "Entity/Entity.hpp"

#include "glm/glm.hpp"

#include <vector>

class World;
class MasterRenderer {
   public:
      MasterRenderer();
      
      std::vector<Renderer *> renderers;

      void activateEntityRenderer(std::vector<Entity> *);
      
      void init();
      void render(const Display &, World *);
      void cleanUp();
};

#endif