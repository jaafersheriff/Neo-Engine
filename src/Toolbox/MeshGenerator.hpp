#pragma once
#ifndef _MESH_GENERATOR_HPP_
#define _MESH_GENERATOR_HPP_

#include "Model/Mesh.hpp"

class MeshGenerator {
    public:
        static Mesh* generateCube(float scale);
        static Mesh* generateSphere(int smoothness);
};

#endif
