#pragma once
#ifndef _SKYBOX_SHADER_HPP_
#define _SKYBOX_SHADER_HPP_

#include "Shader/Shader.hpp"
#include "Skybox/CubeTexture.hpp"

class SkyboxShader : public Shader {
    public:
        SkyboxShader() : Shader("../src/Skybox/SkyboxShader/skybox_vertex_shader.glsl", 
                                "../src/Skybox/SkyboxShader/skybox_fragment_shader.glsl") { }
        
        bool init();

        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);
        void loadCubeTexture(const CubeTexture &);
};

#endif
