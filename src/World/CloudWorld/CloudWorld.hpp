/* Basic world for general development
 * Derives World interface */
#pragma once
#ifndef _CLOUD_WORLD_HPP_
#define _CLOUD_WORLD_HPP_

#include "World/World.hpp"
#include "Toolbox/Toolbox.hpp"

#include "Entity/Entity.hpp"
#include "Cloud/CloudBillboard.hpp"
#include "Sun/Sun.hpp"

class CloudWorld : public World {
    public:
        /* World-specific render targets */
        std::vector<CloudBillboard *> cloudBoards;
        Sun *sun = nullptr;

        /* World-specific members */
        Camera *camera;
        Light *light;
        glm::mat4 P;
        glm::mat4 V;
        bool isPaused = false;

        /* Constructor */
        CloudWorld() : World("Cloud World") { }

        /* Derived functions */
        void init(Context &, Loader &);
        void prepareRenderer(MasterRenderer *);
        void prepareUniforms();
        void update(Context &);
        void takeInput(Mouse &, Keyboard &, const float);
        void cleanUp();
};

#endif
