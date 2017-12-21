#pragma once
#ifndef _ATMOSPHERE_RENDERER_HPP_
#define _ATMOSPHERE_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Atmosphere/AtmosphereShader/AtmosphereShader.hpp"

#include "Atmosphere/Atmosphere.hpp"

class AtmosphereRenderer : public Renderer {
    public:
        Atmosphere *atmosphere;

        bool activate(Atmosphere *);
        void prepare();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void render(const World *);
        void cleanUp();
};

#endif
