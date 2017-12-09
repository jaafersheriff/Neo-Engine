#pragma once 
#ifndef _CLOUD_RENDERER_HPP_
#define _CLOUD_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Cloud/CloudShader/CloudShader.hpp"

#include "Billboard/Billboard.hpp"

class CloudRenderer : public Renderer {
    public:
        std::vector<Billboard *> *billboards;

        bool activate(std::vector<Billboard *> *);
        void prepare();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void render(const World *);
        void cleanUp();
};

#endif