#pragma once
#ifndef _BoundingBox_SHADER_HPP_
#define _BoundingBox_SHADER_HPP_

#include "Renderer/Shader.hpp"
#include "World/LabWorld/Block.hpp"

#include <vector>

class BoundingBoxShader : public Shader {
    public:
        /* Define GLSL shader locations */
        BoundingBoxShader() : Shader("../src/BoundingBox/BoundingBoxShader/boundingBox_vertex_shader.glsl",
                                     "../src/BoundingBox/BoundingBoxShader/boundingBox_fragment_shader.glsl") { }
        /* Reference to render target */
        std::vector<Block *> *blocks;

        /* Init render target and local shaders */
        bool init(std::vector<Block *> *);

        /* Render */
        void render(const World *);

        /* Wrap up and shut down */
        void cleanUp();

        /* Render mesh */
        Mesh *cube;
    private:
        /* GLSL load functions */
        void addAllLocations();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void loadP(const glm::mat4 *);
        void loadV(const glm::mat4 *);
        void loadM(const glm::mat4 *);
        void loadSize(const float);
};

#endif
