// Mesh class
#pragma once
#ifndef _MESH_HPP_
#define _MESH_HPP_

#include <string>
#include <vector>

class Mesh {
    public:
        Mesh();
        void init();

        // Data buffers
        std::vector<float> vertBuf;
        std::vector<float> norBuf;
        std::vector<float> texBuf;
        std::vector<unsigned int> eleBuf;

        // VAO ID
        unsigned int vaoId;

        // VBO ID's
        unsigned int vertBufId;
        unsigned int norBufId;
        unsigned int texBufId;
        unsigned int eleBufId;
};

#endif