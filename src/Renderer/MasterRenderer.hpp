/* Master Renderer class
 * World class activates needed subrenderers which are organized here
 * Master Renderer only renders active subrenderers  */
#pragma once
#ifndef _MASTER_RENDERER_HPP_
#define _MASTER_RENDERER_HPP_

#include "Context/Context.hpp"
#include "Context/Display.hpp"

#include "glm/glm.hpp"

#include <vector>

class World;
class Renderer;
class Entity;
class Skybox;
class CloudBillboard;
class Sun;
class Atmosphere;

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
        void activateAtmosphereRenderer(Atmosphere *);

        /* Init */
        void init(const Context &);

        /* Render function */
        void render(const Display &, const World *);

        /* Wrap up */
        void cleanUp();

        /* Utility functions */
        void toggleWireFrameMode();

    private:
        /* Utility members */
        bool wireFrame = false;
        bool verbose = false;
};

#endif
