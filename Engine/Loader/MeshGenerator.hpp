#pragma once

#include "Loader/Loader.hpp"

namespace neo {

    namespace prefabs {

        void generateCube(MeshData& meshData);
        void generateQuad(MeshData& meshData);
        void generateSphere(MeshData& meshData, int recursions);
        void generatePlane(MeshData& meshData, float h, int VERTEX_COUNT, int numOctaves);
    }
}
