#pragma once
#ifndef _SKYBOX_RENDERER_HPP_
#define _SKYBOX_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Skybox/SkyboxShader/SkyboxShader.hpp"

#include "Skybox/Skybox.hpp"

class SkyboxRenderer : public Renderer {
    public:
        Skybox *skybox;

        void activate(Skybox *);
        void prepare();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void render(const World *);
        void cleanUp();
};

#endif
