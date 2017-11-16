#pragma once
#ifndef _ENTITY_WORLD_HPP_
#define _ENTITY_WORLD_HPP_

#include "World/World.hpp"
#include "Entity/Entity.hpp"
#include "Toolbox/Toolbox.hpp"

class EntityWorld : public World {
public:
      // Render targets
      std::vector<Entity> entities;
      //Skybox *sb;

      // World-specific members
      bool isPaused = false;
      float uTime = 0.f;

      // Constructors
      EntityWorld() : World("Entity World") { }

      // Derived functions
      void init(Loader &);
      void prepareRenderer(MasterRenderer &);
      void update(Context &);
      void takeInput(Mouse &, Keyboard &);
      void cleanUp();
};

#endif
