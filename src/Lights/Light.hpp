#pragma once
#ifndef _LIGHT_HPP_
#define _LIGHT_HPP_

#include "glm/glm.hpp"

class Light {
   public:
      glm::vec3 position = glm::vec3(0, 0, 0);
      glm::vec3 color = glm::vec3(1, 1, 1);
      glm::vec3 attenuation = glm::vec3(1, 0, 0);


      Light(glm::vec3 pos) {
         position = pos;
      }

      Light(glm::vec3 pos, glm::vec3 col) {
         position = pos;
         color = col;
      }

};

#endif