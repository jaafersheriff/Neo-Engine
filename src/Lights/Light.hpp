#pragma once
#ifndef _LIGHT_HPP_
#define _LIGHT_HPP_

#include "glm/glm.hpp"

class Light {
   public:
      glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
      glm::vec3 color = glm::vec3(1.f, 1.f, 1.f);
      glm::vec3 attenuation = glm::vec3(1.f, 0.f, 0.f);

      Light(glm::vec3 p, glm::vec3 c, glm::vec3 a) {
         this->position = p;
         this->color = c;
         this->attenuation = a;
      }

      Light(glm::vec3 pos, glm::vec3 col) {
         Light(pos, col, glm::vec3(1.f, 0.f, 0.f));
      }

      Light(glm::vec3 pos) {
         Light(pos, glm::vec3(1.f, 1.f, 1.f));
      }

      Light() {
         Light(glm::vec3(0.f, 0.f, 0.f));
      }
};

#endif