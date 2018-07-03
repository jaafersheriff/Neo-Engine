/* Atmosphere shader class dervices Shader
 * Contains reference to an atmosphere and entire rendering strategy */
#pragma once
#ifndef _ATMOSPHERE_SHADER_HPP_
#define _ATMOSPHERE_SHADER_HPP_

#include "Renderer/Shader.hpp"

#include "Atmosphere/Atmosphere.hpp"
#include "Light/Light.hpp"
#include "Model/Texture.hpp"

class AtmosphereShader : public Shader {
    public:
        /* Define GLSL shader locations */
        AtmosphereShader() : Shader("../src/Atmosphere/AtmosphereShader/atmosphere_vertex_shader.glsl", 
                                    "../src/Atmosphere/AtmosphereShader/atmosphere_fragment_shader.glsl") { }
                
        /* Reference to render target */
        Atmosphere *atmosphere;

        /* Init render target and local shaders */
        bool init(Atmosphere *);

        /* Render */
        void render();

        /* Wrap up and shut down */
        void cleanUp();

    private:
        /* GLSL load functions */
        void addAllLocations();
        void loadM(const glm::mat4 *);
        void loadColorTexture(const Texture *);
        void loadGlowTexture(const Texture *);
};

#endif
