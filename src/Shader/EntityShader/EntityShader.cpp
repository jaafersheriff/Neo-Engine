#include "EntityShader.hpp"

bool EntityShader::init() {
   if (!Shader::init()) {
      return false;
   }

   // Attributes
   addAttribute("vertexPos");
   addAttribute("vertexNormal");

   // Matrix uniforms
   addUniform("P");
   addUniform("M");
   addUniform("V");

   // Material uniforms
   addUniform("matAmbient");
   addUniform("matDiffuse");
   addUniform("matSpecular");
   addUniform("shine");

   // Light
   addUniform("lightPos");
   addUniform("lightCol");
   addUniform("lightAtt");

   return true;
}

void EntityShader::loadP(const glm::mat4 *p) {
   this->loadMat4(getUniform("P"), p);
}

void EntityShader::loadM(const glm::mat4 *m) {
   this->loadMat4(getUniform("M"), m);
}

void EntityShader::loadV(const glm::mat4 *v) {
   this->loadMat4(getUniform("V"), v);
}

void EntityShader::loadMaterial(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
   this->loadVec3(getUniform("matAmbient"), ambient);
   this->loadVec3(getUniform("matDiffuse"), diffuse);
   this->loadVec3(getUniform("matSpecular"), specular);
}

void EntityShader::loadShine(float s) {
   this->loadFloat(getUniform("shine"), s);
}

void EntityShader::loadReflectivity(float r) {
   this->loadFloat(getUniform("reflectivity"), r);
}

void EntityShader::loadLight(const Light &light) {
   this->loadVec3(getUniform("lightPos"), light.position);
   this->loadVec3(getUniform("lightCol"), light.color);
   this->loadVec3(getUniform("lightAtt"), light.attenuation);
}
