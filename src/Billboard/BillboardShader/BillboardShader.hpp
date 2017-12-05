#pragma once
#ifndef _BILLBOARD_SHADER_HPP_
#define _BILLBOARD_SHADER_HPP_

#include "Shader/Shader.hpp"
#include "Model/Texture.hpp"

class BillboardShader : public Shader {
    public:
        BillboardShader() : Shader("../src/Billboard/BillboardShader/billboard_vertex_shader.glsl",
                                   "../src/Billboard/BillboardShader/billboard_fragment_shader.glsl") { }
        bool init();

        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);
        void loadM(const glm::mat4 *);

        void loadCenter(const glm::vec3);
        void loadSize(const glm::vec2);

        void loadTexture(const Texture *);
};

#endif
