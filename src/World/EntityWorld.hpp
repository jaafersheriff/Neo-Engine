/* Basic world for playing with entities
 * Derives World interface */
#pragma once
#ifndef _ENTITY_WORLD_HPP_
#define _ENTITY_WORLD_HPP_

#include "World/World.hpp"
#include "Entity/Entity.hpp"
#include "Toolbox/Toolbox.hpp"

class EntityWorld : public World {
public:
        /* Render targets */
        std::vector<Entity> entities;
        // TODO : Skybox *sb;

        /* World-specific members */
        bool isPaused = false;

        /* Constructor */
        EntityWorld() : World("Entity World") { }

        /* Derived functions */
        void init(Loader &);
        void prepareRenderer(MasterRenderer &);
        void update(Context &);
        void takeInput(Mouse &, Keyboard &);
        void cleanUp();
};

#endif
