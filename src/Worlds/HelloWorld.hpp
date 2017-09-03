#pragma once
#ifndef _HELLOWORLD_HPP_
#define _HELLOWORLD_HPP_

#include "Worlds/World.hpp"

class HelloWorld : public World {
   public:
      HelloWorld() : World("Hello World") {

      }
      
      void init(/*TODO: Loader*/) {

      }

      void prepareRenderer(/*TODO: MR*/) {

      }

      void update(Context &context) {

      }

      void cleanUp() {

      }

   private:
      void takeInput(Mouse &, Keyboard &) {

      }
};

#endif