#pragma once
#ifndef _TRIANGLE_SHADER_HPP_
#define _TRIANGLE_SHADER_HPP_

#include "Shader/Shader.hpp"

class TriangleShader : public Shader {
   public:
      TriangleShader() : Shader("../src/TriangleWorld/triangle_vertex_shader.glsl", 
                                "../src/TriangleWorld/triangle_fragment_shader.glsl") { }

      bool init();

      void loadMVP(const glm::mat4);
};

#endif