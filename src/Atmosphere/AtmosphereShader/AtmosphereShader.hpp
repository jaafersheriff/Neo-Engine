#pragma once
#ifndef _ATMOSPHERE_SHADER_HPP_
#define _ATMOSPHERE_SHADER_HPP_

#include "Shader/Shader.hpp"

#include "Light/Light.hpp"
#include "Model/Texture.hpp"

class AtmosphereShader : public Shader {
    public:
        AtmosphereShader() : Shader("../src/Atmosphere/AtmosphereShader/atmosphere_vertex_shader.glsl", 
                                    "../src/Atmosphere/AtmosphereShader/atmosphere_fragment_shader.glsl") { }
                
        bool init();

        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);

        void loadLight(const Light *);

        void loadColorTexture(const Texture *);
        void loadGlowTexture(const Texture *);

};

#endif
