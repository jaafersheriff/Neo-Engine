// Model Texture class
// Contains loaded texture, texture properties, and material properties
// Rendering will based on material properties if loaded texture doesn't exist
#pragma once
#ifndef _MODEL_TEXTURE_HPP_
#define _MODEL_TEXTURE_HPP_

#include "Toolbox/Loader.hpp"

#include "glm/glm.hpp"

#include <string>

// TODO: comments explaining each texture/material property
class ModelTexture {
   public:
      void loadTexture(Loader *loader, std::string fileName) {
         this->textureId = loader->loadPngTexture(fileName);
      }

      // Texture properties
      GLint textureId = 0;
      float shineDamper = 1.f;
      float reflectivity = 0.f;
      bool hasTranspency = false;
      bool hasFakeLighting = false;
      int numRows = 1;

      // Material properties
      glm::vec3 ambientColor = glm::vec3(0, 0, 0);
      glm::vec3 diffuseColor = glm::vec3(0, 0, 0);
      glm::vec3 specularColor = glm::vec3(0, 0, 0);
};

#endif