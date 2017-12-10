/* Master Renderer class
 * World class activates needed subrenderers which are organized here
 * Master Renderer only renders active subrenderers  */
#pragma once
#ifndef _MASTER_RENDERER_HPP_
#define _MASTER_RENDERER_HPP_

#include "Context/Display.hpp"

#include "glm/glm.hpp"

#include <vector>

class World;
class Renderer;
class Entity;
class Skybox;
class CloudBillboard;
class Sun;

class MasterRenderer {
    public:
        /* Constructor */
        MasterRenderer();
        
        /* List of active renderers */
        std::vector<Renderer *> renderers;

        /* Activate subrenderers */
        void activateEntityRenderer(std::vector<Entity *> *);
        void activateCloudRenderer(std::vector<CloudBillboard *> *);
        void activateSkyboxRenderer(Skybox *);
        void activateSunRenderer(Sun *);
        
        /* Init */
        void init();

        /* Render function */
        void render(const Display &, const World *);

        /* Wrap up */
        void cleanUp();

        /* Utility functions */
        bool wireFrame = false;
        void toggleWireFrameMode();
};

#endif
