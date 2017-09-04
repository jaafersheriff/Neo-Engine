#pragma once
#ifndef _HELLOWORLD_HPP_
#define _HELLOWORLD_HPP_

#include "Triangle.hpp"
#include "World/World.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GLFW/glfw3.h>

class TriangleWorld : public World {
   public:
      Triangle t;

      TriangleWorld() : World("Triangle World") { }
      
      void init(/*TODO: Loader*/) {

         glGenVertexArrays(1, &t.vaoId);
         glBindVertexArray(t.vaoId);
         // send positions to GPU
         glGenBuffers(1, &t.posId);
         glBindBuffer(GL_ARRAY_BUFFER, t.posId);
         glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float), &t.posBuf[0], 
           GL_STATIC_DRAW);
         // send colors to GPU
         glGenBuffers(1, &t.colId);
         glBindBuffer(GL_ARRAY_BUFFER, t.colId);
         glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), &t.colBuf[0], 
           GL_STATIC_DRAW);
      }

      void prepareRenderer(MasterRenderer &mr) {
         mr.activateTriangleRenderer(&t);
      }

      void update(Context &context) {
         glm::mat4 m(1.f);
         m = glm::rotate(m, (float)glfwGetTime(), glm::vec3(0, 0, 1));
         float ratio = context.display.width/(float)context.display.height;
         glm::mat4 p = glm::ortho(-ratio, ratio, -1.f, 1.f);
         t.mvp = m*p;
      }

      void cleanUp() {

      }

   private:
      void takeInput(Mouse &, Keyboard &) {

      }
};

#endif