#pragma once 
#ifndef _BILLBOARD_RENDERER_HPP_
#define _BILLBOARD_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Billboard/BillboardShader/BillboardShader.hpp"

#include "Billboard/Billboard.hpp"

class BillboardRenderer : public Renderer {
    public:
        std::vector<Billboard *> *billboards;

        void activate(std::vector<Billboard *> *);
        void prepare();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void render(const World *);
        void cleanUp();
    private:
        void sort();
};

#endif