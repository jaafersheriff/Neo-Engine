#include "TriangleShader.hpp"

bool TriangleShader::init() {
   if (!Shader::init()) {
      return false;
   }

   addUniform("mvp");   // mat4

   addAttribute("vCol");   // vec3
   addAttribute("vPos");   // vec2

   return true;
}

void TriangleShader::loadMVP(const glm::mat4 mvp) {
   loadMat4(getUniform("mvp"), mvp);
}
