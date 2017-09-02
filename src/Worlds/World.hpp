// Abstract parent World class
#pragma once
#ifndef _WORLD_HPP_
#define _WORLD_HPP_

class World {
   public:
      // Abstract types
      std::string name;
      // TODO: Camera camera;
      // TODO: std::vector<Light> lights;

      // Abstract functions
      void init(/* TODO: Loader loader*/);
      void prepareRenderer(/* TODO: &MasterRenderer */);
      void update(Context&);

   private:
      takeInput(Mouse&, Keyboard&)
};

#endif