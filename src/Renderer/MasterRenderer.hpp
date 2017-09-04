// Master Renderer class
// Contains all subrenderers
// World class activates different subrenderers
// Master Renderer only renders active subrenderers 
#pragma once
#ifndef _MASTER_RENDERER_HPP_
#define _MASTER_RENDERER_HPP_

#include "TriangleWorld/TriangleRenderer.hpp"

#include "glm/glm.hpp"

class MasterRenderer {
   public:
      std::vector<Renderer *> renderers;

      void activateTriangleRenderer(Triangle *t);
      void render();
};

#endif