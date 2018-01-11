/* Basic world for general development
 * Derives World interface */
#pragma once
#ifndef _DEV_WORLD_HPP_
#define _DEV_WORLD_HPP_

#include "World/World.hpp"
#include "Toolbox/Toolbox.hpp"

#include "Entity/Entity.hpp"
#include "Cloud/CloudBillboard.hpp"
#include "Sun/Sun.hpp"

class DevWorld : public World {
    public:
        /* World-specific render targets */
        std::vector<Entity *> entities;
        std::vector<CloudBillboard *> cloudBoards;
        Sun *sun = nullptr;

        /* World-specific members */
        bool isPaused = false;

        /* Constructor */
        DevWorld() : World("Dev World") { }

        /* Derived functions */
        void init(Loader &);
        void prepareRenderer(MasterRenderer *);
        void update(Context &);
        void takeInput(Mouse &, Keyboard &);
        void cleanUp();
};

#endif
