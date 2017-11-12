#include "Skybox.hpp"

Skybox::Skybox(const std::string textureNames[6]) {
   /* Init mesh */
   mesh = new Mesh;
   mesh->vertBuf = this->verts;
   mesh->init();

   /* TODO : Init cube map textures */
}