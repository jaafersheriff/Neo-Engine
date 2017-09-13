// Mesh class
#pragma once
#ifndef _MESH_HPP_
#define _MESH_HPP_

#include <vector>

class Mesh {
   public:
      Mesh();
      void init();

      std::vector<float> vertBuf;
      std::vector<float> norBuf;
      std::vector<float> texBuf;
      std::vector<unsigned int> eleBuf;

   protected:
      unsigned int vaoId;
      unsigned int vertBufId;
      unsigned int norBufId;
      unsigned int texBufId;
      unsigned int eleBufId;
};

#endif