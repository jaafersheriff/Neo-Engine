#pragma once
#ifndef _AABB_SHADER_HPP_
#define _AABB_SHADER_HPP_

#include "Renderer/Shader.hpp"
#include "World/LabWorld/Block.hpp"

#include <vector>

class AABBShader : public Shader {
    public:
        /* Define GLSL shader locations */
        AABBShader() : Shader("../src/Entity/AABB/AABBShader/aabb_vertex_shader.glsl",
                              "../src/Entity/AABB/AABBShader/aabb_fragment_shader.glsl") { }
        /* Reference to render target */
        std::vector<Block *> *blocks;

        /* Init render target and local shaders */
        bool init(std::vector<Block *> *);

        /* Render */
        void render(const World *);

        /* Wrap up and shut down */
        void cleanUp();

    private:
        /* AABB VAO/VBO */
        float vertices[24];
        unsigned int vaoId;
        unsigned int vertexId;
        
        /* GLSL load functions */
        void addAllLocations();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);
};

#endif
