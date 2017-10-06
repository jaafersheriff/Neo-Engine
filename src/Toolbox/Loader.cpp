#include "Loader.hpp"
#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>
#include <vector>

Texture Loader::loadPngTexture(const std::string fileName) {
   Texture texture;
   std::map<std::string, GLint>::iterator it = textures.find(fileName);
   if (it != textures.end()) {
      texture.textureId = it->second;
   }
   else {
      char *data; // TODO : load in a png image
      int sizeX, sizeY;

      texture.init(sizeX, sizeY, data);
      textures.insert(std::map<std::string, GLint>::value_type(fileName, texture.textureId));
   }
   return texture;
}

// Load geometry from .obj
Mesh* Loader::loadObjMesh(const std::string fileName) {
   std::map<std::string, Mesh*>::iterator it = meshes.find(fileName);
   if (it != meshes.end()) {
      return it->second;
   }

   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> objMaterials;
   std::string errString;
   bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, fileName.c_str());
   if (!rc) {
      std::cerr << errString << std::endl;
      exit(1);
   }
      
   //////////////////////////// NOTICE ////////////////////////////////
   // tiny_obj_loader creates meshes for every shape in the obj file //
   // This function will return only the first loaded mesh           //
   // TODO: Expand this function to return all the avilable meshes   //
   ////////////////////////////////////////////////////////////////////
   Mesh *mesh = new Mesh;
   mesh->name = fileName;
   mesh->vertBuf = shapes[0].mesh.positions;
   mesh->norBuf  = shapes[0].mesh.normals;
   mesh->texBuf  = shapes[0].mesh.texcoords;
   mesh->eleBuf  = shapes[0].mesh.indices; 
   resize(mesh);
   mesh->init();
   meshes.insert(std::map<std::string, Mesh*>::value_type(fileName, mesh));

   std::cout << "Loaded mesh (" << mesh->vertBuf.size()/3 << "): " << fileName << std::endl;

   return mesh;
}

// Provided function to resize a mesh so all vertex positions are [0, 1.f]
void Loader::resize(Mesh *mesh) {
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < mesh->vertBuf.size() / 3; v++) {
      if(mesh->vertBuf[3*v+0] < minX) minX = mesh->vertBuf[3*v+0];
      if(mesh->vertBuf[3*v+0] > maxX) maxX = mesh->vertBuf[3*v+0];

      if(mesh->vertBuf[3*v+1] < minY) minY = mesh->vertBuf[3*v+1];
      if(mesh->vertBuf[3*v+1] > maxY) maxY = mesh->vertBuf[3*v+1];

      if(mesh->vertBuf[3*v+2] < minZ) minZ = mesh->vertBuf[3*v+2];
      if(mesh->vertBuf[3*v+2] > maxZ) maxZ = mesh->vertBuf[3*v+2];
   }

   //From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
   maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
	for (size_t v = 0; v < mesh->vertBuf.size() / 3; v++) {
		mesh->vertBuf[3*v+0] = (mesh->vertBuf[3*v+0] - shiftX) * scaleX;
		assert(mesh->vertBuf[3*v+0] >= -1.0 - epsilon);
		assert(mesh->vertBuf[3*v+0] <= 1.0 + epsilon);
		mesh->vertBuf[3*v+1] = (mesh->vertBuf[3*v+1] - shiftY) * scaleY;
		assert(mesh->vertBuf[3*v+1] >= -1.0 - epsilon);
		assert(mesh->vertBuf[3*v+1] <= 1.0 + epsilon);
		mesh->vertBuf[3*v+2] = (mesh->vertBuf[3*v+2] - shiftZ) * scaleZ;
		assert(mesh->vertBuf[3*v+2] >= -1.0 - epsilon);
		assert(mesh->vertBuf[3*v+2] <= 1.0 + epsilon);
	}
}