/* Abstract parent World class */
#pragma once
#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "Renderer/MasterRenderer.hpp"
#include "Context/Context.hpp"
#include "Camera/Camera.hpp"
#include "Light/Light.hpp"
#include "Toolbox/Loader.hpp"

#include <string>
#include <vector>

class World {
    public:
        /* Include world-specific data structure to be rendered  */
        World(const std::string n) : name(n) { }
        std::string name;

        /* World members */
        Camera *camera;
        Light *light;

        /* Create objects, initialize rendering data structure */
        virtual void init(Loader &) = 0;

        /* Activate feature renderers in MasterRenderer and pass in proper
         * data structure */
        virtual void prepareRenderer(MasterRenderer *) = 0;

        /* Update features, call takeInput() */
        virtual void update(Context &) = 0;

        /* Any necessary clean up */
        virtual void cleanUp() = 0;

    private:
        /* Process any user input */
        virtual void takeInput(Mouse &, Keyboard &) = 0;
};

#endif
