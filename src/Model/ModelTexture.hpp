// Model Texture class
// Contains reference to an optionally loaded texture, texture properties, and material properties
// Rendering will based on material properties if loaded texture doesn't exist
#pragma once
#ifndef _MODEL_TEXTURE_HPP_
#define _MODEL_TEXTURE_HPP_

#include "Texture.hpp"

#include "glm/glm.hpp"

#include <string>

// TODO: comments explaining each texture/material property
class ModelTexture {
   public:
      // Texture properties
      Texture textureImage;
      float shineDamper = 1.f;
      bool hasTranspency = false;
      bool hasFakeLighting = false;
      int numRows = 1;

      // Material properties
      glm::vec3 ambientColor = glm::vec3(0.f, 0.f, 0.f);
      glm::vec3 diffuseColor = glm::vec3(0.f, 0.f, 0.f);
      glm::vec3 specularColor = glm::vec3(1.f, 1.f, 1.f);
};

#endif