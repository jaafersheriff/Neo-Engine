#pragma once
#ifndef _BOUNDING_SPHERE_SHADER_HPP_
#define _BOUNDING_SPHERE_SHADER_HPP_

#include "Collision/BoundingSphere.hpp"
#include "Renderer/Shader.hpp"

#include <vector>

class BoundingSphereShader : public  Shader {
    public:
        /* Define GLSL Shader locations */
        BoundingSphereShader() : Shader("../src/Collision/BoundingSphereShader/bounding_sphere_vertex_shader.glsl",
                                        "../src/Collision/BoundingSphereShader/bounding_sphere_fragment_shader.glsl") { }

        /* Reference to render target */
        std::vector<BoundingSphere *> *spheres;

        /* Init render target and local shaders */
        bool init(std::vector<BoundingSphere *> *);

        /* Render */
        void render();

        /* Wrap up and shut down */
        void cleanUp();
    private:
        /* Sphere mesh */
        Mesh *sphereMesh;

        /* GLSL Load functions */
        void addAllLocations();
        void loadM(const glm::mat4 *);
};

#endif
