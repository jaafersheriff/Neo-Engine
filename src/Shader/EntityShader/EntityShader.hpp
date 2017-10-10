#pragma once
#ifndef _ENTITY_SHADER_HPP_
#define _ENTITY_SHADER_HPP_

#include "Shader/Shader.hpp"
#include "Lights/Light.hpp"
#include "Model/Texture.hpp"

class EntityShader : public Shader {
   public:
      EntityShader() : Shader("../src/Shader/EntityShader/entity_vertex_shader.glsl",
                              "../src/Shader/EntityShader/entity_fragment_shader.glsl") { }
      
      bool init();

      void loadP(const glm::mat4 *);
      void loadM(const glm::mat4 *);
      void loadV(const glm::mat4 *);
      void loadMaterial(glm::vec3, glm::vec3, glm::vec3);
      void loadShine(float);
      void loadLight(const Light &);
      void loadUsesTexture(const bool);
      void loadTexture(const Texture &);
};

#endif