/* Atmosphere shader class dervices Shader
 * Contains reference to an atmosphere and entire rendering strategy */
#pragma once
#ifndef _ATMOSPHERE_SHADER_HPP_
#define _ATMOSPHERE_SHADER_HPP_

#include "Shader/Shader.hpp"

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
        void render(const World *);

        /* Wrap up and shut down */
        void cleanUp();

    private:
        /* GLSL load functions */
        void addAllLocations();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);
        void loadM(const glm::mat4 *);
        void loadLight(const Light *);
        void loadColorTexture(const Texture *);
        void loadGlowTexture(const Texture *);
};

#endif
