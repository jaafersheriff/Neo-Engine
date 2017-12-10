#pragma once
#ifndef _SUN_RENDERER_HPP_
#define _SUN_RENDERER_HPP_

#include "Renderer/Renderer.hpp"
#include "Sun/SunShader/SunShader.hpp"

#include "Sun/Sun.hpp"

class SunRenderer : public Renderer {
    public:
        Sun *sun;

        bool activate(Sun *);
        void prepare();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void render(const World *);
        void cleanUp();
};

#endif
