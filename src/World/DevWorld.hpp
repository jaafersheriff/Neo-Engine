/* Basic world for general development
 * Derives World interface */
#pragma once
#ifndef _DEV_WORLD_HPP_
#define _DEV_WORLD_HPP_

#include "World/World.hpp"
#include "Entity/Entity.hpp"
#include "Toolbox/Toolbox.hpp"

class DevWorld : public World {
public:
        /* Render targets */
        std::vector<Entity> entities;
        Skybox sb;

        /* World-specific members */
        bool isPaused = false;

        /* Constructor */
        DevWorld() : World("Entity World") { }

        /* Derived functions */
        void init(Loader &);
        void prepareRenderer(MasterRenderer *);
        void update(Context &);
        void takeInput(Mouse &, Keyboard &);
        void cleanUp();
};

#endif
