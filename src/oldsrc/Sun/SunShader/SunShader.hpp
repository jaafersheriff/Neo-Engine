/* Sun Shader derives Shader 
 * Contains reference to a sun and entire rendering strategy */
#pragma once
#ifndef _SUN_SHADER_CPP_
#define _SUN_SHADER_CPP_

#include "Renderer/Shader.hpp"
#include "Sun/Sun.hpp"
#include "Model/Texture.hpp"

class SunShader : public Shader {
    public:
        /* Define GLSL shader locations */
        SunShader() : Shader("../src/Sun/SunShader/sun_vertex_shader.glsl",
                             "../src/Sun/SunShader/sun_fragment_shader.glsl") { }
        
        /* Reference to render target*/
        Sun *sun;

        /* Init render target and local shaders */
        bool init(Sun *);

        /* Render */
        void render();

        /* Wrap up and shut down */
        void cleanUp();

    private:
        /* GLSL load functions */
        void addAllLocations();
        void loadCenter(const glm::vec3);
        void loadSize(const glm::vec2);
        void loadUsesTexture(bool);
        void loadTexture(const Texture *);
        void loadInnerColor(glm::vec3);
        void loadOuterColor(glm::vec3);
        void loadInnerRadius(float);
        void loadOuterRadius(float);
};

#endif