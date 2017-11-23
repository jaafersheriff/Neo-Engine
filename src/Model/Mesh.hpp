/* Mesh class
 * Contains geometry for a singular mesh */
#pragma once
#ifndef _MESH_HPP_
#define _MESH_HPP_

#include <vector>   /* vector */

class Mesh {
    public:
        /* Constructor */
        Mesh();

        /* Copy data buffers to GPU */
        void init();

        /* Data buffers */
        std::vector<float> vertBuf;
        std::vector<float> norBuf;
        std::vector<float> texBuf;
        std::vector<unsigned int> eleBuf;

        /* VAO ID */
        unsigned int vaoId;

        /* VBO IDs */
        unsigned int vertBufId;
        unsigned int norBufId;
        unsigned int texBufId;
        unsigned int eleBufId;
};

#endif
