#pragma once
#ifndef _SUN_SHADER_CPP_
#define _SUN_SHADER_CPP_

#include "Shader/Shader.hpp"
#include "Model/Texture.hpp"

class SunShader : public Shader {
    public:
        SunShader() : Shader("../src/Sun/SunShader/sun_vertex_shader.glsl",
                             "../src/Sun/SunShader/sun_fragment_shader.glsl") { }
        
        bool init();

        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);

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