#pragma once
#ifndef _MESH_GENERATOR_HPP_
#define _MESH_GENERATOR_HPP_

#include "Model/Mesh.hpp"

class MeshGenerator {
    public:
        /* Generate shape meshes */
        static Mesh* generateCube(float scale);
        static Mesh* generateSphere(int smoothness);

        /* Resize a mesh so all of the vertices are [0, 1] */
        static void resize(Mesh *);
};

#endif
