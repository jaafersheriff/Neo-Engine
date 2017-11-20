#pragma once
#ifndef _ENTITY_RENDERER_HPP_
#define _ENTITY_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Entity/EntityShader/EntityShader.hpp"

#include "Entity/Entity.hpp"

#include <vector>

class EntityRenderer : public Renderer {
    public:
        std::vector<Entity> *entitiesPointer;

        void activate(std::vector<Entity> *);
        void prepare();
        void setGlobals(const glm::mat4*, const glm::mat4*);
        void render(const World *);
        void cleanUp();
    private:
        void prepareMesh(const Mesh *);
        void prepareTexture(const ModelTexture &);
        void unPrepareMesh(const Mesh *);
        void unPrepareTexture(const ModelTexture &);
};

#endif