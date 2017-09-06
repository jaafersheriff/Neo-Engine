// Renderer abstract parent class
#pragma once
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "Shader/Shader.hpp"

class Renderer {
   public:
      Shader *shader;

      virtual void setGlobals(/* TODO: MR **/) = 0;
      virtual void render() = 0;
};

#endif
