#pragma once 
#ifndef _CLOUD_RENDERER_HPP_
#define _CLOUD_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Cloud/CloudShader/CloudShader.hpp"

#include "Cloud/CloudBillboard.hpp"

class CloudRenderer : public Renderer {
    public:
        std::vector<CloudBillboard *> *billboards;

        bool activate(std::vector<CloudBillboard *> *);
        void prepare();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void render(const World *);
        void cleanUp();
};

#endif
