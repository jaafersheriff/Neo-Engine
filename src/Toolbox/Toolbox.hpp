#pragma once
#ifndef _TOOLBOX_HPP_
#define _TOOLBOX_HPP_

#include <stdlib.h>

class Toolbox {
public:
   static constexpr double PI = 3.14159265359;

   static inline float genRandom() {
      return rand() / (float) RAND_MAX;
   }
};


#endif