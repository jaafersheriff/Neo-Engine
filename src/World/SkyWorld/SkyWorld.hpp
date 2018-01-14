/* Sky world 
 * Dervices World interface
 * Used to experiment with different sky objects:
 *  Skybox
 *  Sun
 *  Atmosphere */
#pragma once
#ifndef _SKY_WORLD_HPP_
#define _SKY_WORLD_HPP_

#include "World/World.hpp"
#include "Skybox/Skybox.hpp"
#include "Sun/Sun.hpp"
#include "Atmosphere/Atmosphere.hpp"

class SkyWorld : public World {
    public:
        /* World-specific render targets */
        Skybox *skybox = nullptr;
        Sun *sun = nullptr;
        Atmosphere *atmosphere = nullptr;

        /* World-specific members */
        glm::mat4 P;
        glm::mat4 V;

        /* Constructor */
        SkyWorld() : World("Sky World") { }

        /* Derived functions */
        void init(Context &, Loader &);
        void prepareRenderer(MasterRenderer *);
        void prepareUniforms();
        void update(Context &);
        void takeInput(Mouse &, Keyboard &);
        void cleanUp();
};

#endif
