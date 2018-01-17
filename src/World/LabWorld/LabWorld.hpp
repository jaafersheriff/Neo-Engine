#pragma once
#ifndef _LAB_WORLD_HPP_
#define _LAB_WORLD_HPP_

#include "World/World.hpp"
#include "Entity/Entity.hpp"
#include "Player.hpp"
#include "Block.hpp"

#define MAX_GAME_OBJECTS 15

class LabWorld : public World {
    public:
        /* World-specific members */
        Player *player;

        /* World-specific render targets */
        std::vector<Entity *> entities;
        std::vector<Block *> blocks;

        /* Constructor */
        LabWorld() : World("CSC 476 Lab") { }

        /* Derived functions */
        void init(Loader &);
        void prepareRenderer(MasterRenderer *);
        void update(Context &);
        void takeInput(Mouse &, Keyboard &);
        void cleanUp();
    private:
        Loader *loader;
        int gameObjects = 0;
        ModelTexture alive = ModelTexture(0.3f, glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 1.f));
};

#endif
