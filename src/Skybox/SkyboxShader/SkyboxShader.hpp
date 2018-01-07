/* Skybox Shader class dervices Shader 
 * Contains reference to skybox and entire rendering strategy */
#pragma once
#ifndef _SKYBOX_SHADER_HPP_
#define _SKYBOX_SHADER_HPP_

#include "Shader/Shader.hpp"
#include "Skybox/Skybox.hpp"
#include "Skybox/CubeTexture.hpp"

class SkyboxShader : public Shader {
    public:
        /* Define GLSL shader locations */
        SkyboxShader() : Shader("../src/Skybox/SkyboxShader/skybox_vertex_shader.glsl", 
                                "../src/Skybox/SkyboxShader/skybox_fragment_shader.glsl") { }
        
        /* Reference to render target */
        Skybox *skybox;

        /* Init render target and local shaders */
        bool init(Skybox *);

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
        void loadCubeTexture(const CubeTexture *);
};

#endif
