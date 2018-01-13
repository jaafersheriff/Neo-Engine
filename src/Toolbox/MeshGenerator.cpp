#include "MeshGenerator.hpp"

Mesh* generateCube(float scale) {
    Mesh *mesh = new Mesh;
    mesh->vertBuf = { 
            -scale,  scale, -scale,
            -scale, -scale, -scale,
             scale, -scale, -scale,
             scale, -scale, -scale,
             scale,  scale, -scale,
            -scale,  scale, -scale,
            
            -scale, -scale,  scale,
            -scale, -scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale,  scale,
            -scale, -scale,  scale,
            
             scale, -scale, -scale,
             scale, -scale,  scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
             scale,  scale, -scale,
             scale, -scale, -scale,
            
            -scale, -scale,  scale,
            -scale,  scale,  scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
             scale, -scale,  scale,
            -scale, -scale,  scale,
            
            -scale,  scale, -scale,
             scale,  scale, -scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
            -scale,  scale,  scale,
            -scale,  scale, -scale,
            
            -scale, -scale, -scale,
            -scale, -scale,  scale,
             scale, -scale, -scale,
             scale, -scale, -scale,
            -scale, -scale,  scale,
             scale, -scale,  scale
        };

    return mesh;
}