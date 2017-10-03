#pragma once
#ifndef _ENTITY_WORLD_HPP_
#define _ENTITY_WORLD_HPP_

#include "World/World.hpp"
#include "Entity/Entity.hpp"
#include "Toolbox/Toolbox.hpp"

class EntityWorld : public World {
   public:
      std::vector<Entity> entities;
      bool isPaused = false;

      EntityWorld() : World("Entity World") { }

      void init(Loader &);
      void prepareRenderer(MasterRenderer &);
      void update(Context &);
      void takeInput(Mouse &, Keyboard &);
      void cleanUp();
};

#endif