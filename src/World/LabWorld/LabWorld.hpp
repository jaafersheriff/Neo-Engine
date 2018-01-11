#pragma once
#ifndef _LAB_WORLD_HPP_
#define _LAB_WORLD_HPP_

#include "World/World.hpp"
#include "Entity/Entity.hpp"

class LabWorld : public World {
    public:
        /* World-specific render targets */
        std::vector<Entity *> entities;

        /* Constructor */
        LabWorld() : World("CSC 476 Lab") { }

        /* Derived functions */
        void init(Loader &);
        void prepareRenderer(MasterRenderer *);
        void update(Context &);
        void takeInput(Mouse &, Keyboard &);
        void cleanUp();
};

#endif
