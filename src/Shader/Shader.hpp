// Shader abstract parent class
#pragma once
#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <map>
#include <string>

class Shader {
   public:
      Shader(std::string v = "", std::string f = "") : vShaderName(v), fShaderName(f) { }

      GLuint pid = 0;

      virtual bool init();
      virtual GLuint createShader(std::string, GLenum);
      virtual void bind();
      virtual void unbind();

      void addAttribute(const std::string &);
      void addUniform(const std::string &);
      GLint getAttribute(const std::string &);
      GLint getUniform(const std::string &);

      void loadFloat(int location, float);
      void loadVec2(int location, glm::vec2);
      void loadVec3(int location, glm::vec3);
      void loadMat4(int location, glm::mat4);

   protected:
      const std::string vShaderName;
      const std::string fShaderName;
   
   private:
      std::map<std::string, GLint> attributes;
      std::map<std::string, GLint> uniforms;
};

#endif