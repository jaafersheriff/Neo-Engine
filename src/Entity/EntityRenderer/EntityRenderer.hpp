/* Entity Renderer class dervices Renderer
 * Contains reference to list of entities to be rendered */
#pragma once
#ifndef _ENTITY_RENDERER_HPP_
#define _ENTITY_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Entity/EntityShader/EntityShader.hpp"

#include "Entity/Entity.hpp"

#include <vector>   /* vector */

class EntityRenderer : public Renderer {
    public:
        /* Data structure to be rendered */
        std::vector<Entity> *entitiesPointer;

        /* Activate self and shader */
        void activate(std::vector<Entity> *);

        /* Set any global variables in shader */
        void setGlobals(const glm::mat4*, const glm::mat4*);

        /* Called before render() */
        void prepare();

        /* Render entities */
        void render(const World *);

        /* Wrap up on shut down */
        void cleanUp();
    private:
        /* Bind mesh */
        void prepareMesh(const Mesh *);

        /* Unbind mesh */
        void unPrepareMesh(const Mesh *);
        
        /* Bind texture */
        void prepareTexture(const ModelTexture &);

        /* Unbind texture */
        void unPrepareTexture(const ModelTexture &);
};

#endif
