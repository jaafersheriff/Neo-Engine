#pragma once
#ifndef _TRIANGLE_HPP_
#define _TRIANGLE_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include "glm/glm.hpp"

#include <vector>

class Triangle {
   public:
      GLuint vaoId;
      GLuint posId;
      GLuint colId;

      float posBuf[6] = {
         0.6f, 0.4f,
         -0.6f, 0.4f,
         0.f, -0.4f
      };
      float colBuf[9] = {
         1.f, 0.f, 0.f,
         0.f, 1.f, 0.f,
         0.f, 0.f, 1.f
      };

      glm::mat4 mvp;
};

#endif