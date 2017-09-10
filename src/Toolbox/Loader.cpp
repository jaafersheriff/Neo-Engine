#include "Loader.hpp"

GLint Loader::loadPngTexture(const std::string fileName) {
   if (textures.keys().contains(filename)) {
      return textures.get(filename);
   }

   // TODO
}

// Load geometry from .obj
Mesh* Loader::loadObjMesh(const std::string fileName) {
   if (meshes.keys().contains(fileName)) {
      return meshes.get(fileName);
   }

   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> objMaterials;
   std::string errorString;
   bool rc = tinyobj::LoadObj(shapes, materials, errString, filename.c_str());
   if (!rc) {
      std::cerr << errString << std::endl;
      exit(1);
   }
      
   // NOTICE - MULTIPLE MESHES CAN BE LOADED FOR A SINGLE OBJ
   //          THIS FUNCTION WILL ONLY CREATE AND RETURN ONE MESH
   Mesh *mesh = new Mesh;
   mesh->vertBuf = shapes[0].mesh.positions;
   mesh->norBuf  = shapes[0].mesh.normals;
   mesh->texBuf  = shapes[0].mesh.texcoords;
   mesh->eleBuf  = shapes[0].mesh.indices;
   meshes.put(filename, mesh);
   return mesh;
}

// Provided function to resize a mesh so all vertex positions are [0, 1.0]
void Loader::resize(Mesh *mesh) {
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
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
	for (size_t v = 0; v < posBuf.size() / 3; v++) {
		posBuf[3*v+0] = (posBuf[3*v+0] - shiftX) * scaleX;
		assert(posBuf[3*v+0] >= -1.0 - epsilon);
		assert(posBuf[3*v+0] <= 1.0 + epsilon);
		posBuf[3*v+1] = (posBuf[3*v+1] - shiftY) * scaleY;
		assert(posBuf[3*v+1] >= -1.0 - epsilon);
		assert(posBuf[3*v+1] <= 1.0 + epsilon);
		posBuf[3*v+2] = (posBuf[3*v+2] - shiftZ) * scaleZ;
		assert(posBuf[3*v+2] >= -1.0 - epsilon);
		assert(posBuf[3*v+2] <= 1.0 + epsilon);
	}
}