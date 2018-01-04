#pragma once
#ifndef _CLOUD_SHADER_HPP_
#define _CLOUD_SHADER_HPP_

#include "Shader/Shader.hpp"
#include "Model/Texture.hpp"
#include "Light/Light.hpp"

class CloudShader : public Shader {
    public:
        CloudShader() : Shader("../src/Cloud/CloudShader/cloud_vertex_shader.glsl",
                               "../src/Cloud/CloudShader/cloud_fragment_shader.glsl") { }
        bool init();

        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);
        void loadM(const glm::mat4 *);

        void loadCenter(const glm::vec3);
        void loadSize(const glm::vec2);

        void loadTexture(const Texture *);

        void loadLight(const Light *);
};

#endif
