#pragma once
#ifndef _MESH_GENERATOR_HPP_
#define _MESH_GENERATOR_HPP_

#include "Model/Mesh.hpp"

class MeshGenerator {
    public:
        Mesh* generateCube(float scale);
};

#endif
