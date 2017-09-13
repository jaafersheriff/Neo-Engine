// Loader class - loads meshes and textures
// Contains a collection of textures and meshes so they are only loaded once 
#pragma once
#ifndef _LOADER_HPP_
#define _LOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <string> // file name string
#include <map>    

class Mesh;
class Loader {
   public:
      GLint loadPngTexture(const std::string);
      Mesh* loadObjMesh(const std::string);
      void resize(Mesh*);
      
   private:
      // Collections that prevent loading textures/meshes more than once 
      std::map<std::string, GLint> textures;
      std::map<std::string, Mesh*> meshes;
};

#endif