// Loader class - loads meshes and textures
// Contains a collection of textures and meshes so they are only loaded once 
#pragma once
#ifndef _LOADER_HPP_
#define _LOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <map>    
#include <vector>

class Mesh;
class Texture;
class Loader {
   public:
      Texture loadTexture(const std::string);
      Mesh* loadObjMesh(const std::string);
      
   private:
      void resize(Mesh*);
      // Collections that prevent loading textures/meshes more than once 
      std::map<std::string, GLint> textures;
      std::map<std::string, Mesh*> meshes;
};

#endif