// Derived renderer
// Used for testing -- will likely delete
#pragma once
#ifndef _TRI_RENDERER_HPP_
#define _TRI_RENDERER_HPP_

#include "Triangle.hpp"
#include "Renderer/Renderer.hpp"
#include "TriangleShader.hpp"
#include "glm/glm.hpp"

#include <vector>

class TriangleRenderer : public Renderer {
   public:
      Triangle *tri;
      TriangleShader *shader;
      void activate(Triangle *);
      void setGlobals();
      void render();
};

#endif