/////////////////////////////////////////////////////////////////////////////////////////
//                           Abstract parent Shader class                              //
//                                                                                     //
// How to create a derived shader:                                                     //
//    Constructor(): call parent constructor with shader names                         //
//    init(): call parent constructor, add uniforms, add attributes                    //
//    shader-specific load functions                                                   //
/////////////////////////////////////////////////////////////////////////////////////////
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
      GLint vShaderId;
      GLint fShaderId;

      virtual bool init();
      virtual GLuint createShader(std::string, GLenum);
      virtual void bind();
      virtual void unbind();

      void addAttribute(const std::string &);
      void addUniform(const std::string &);
      GLint getAttribute(const std::string &);
      GLint getUniform(const std::string &);

      void loadFloat(const int, const float) const;
      void loadVec2(const int, const glm::vec2) const;
      void loadVec3(const int, const glm::vec3) const;
      void loadMat4(const int, const glm::mat4*) const;

      void cleanUp();

   protected:
      const std::string vShaderName;
      const std::string fShaderName;
   
   private:
      std::map<std::string, GLint> attributes;
      std::map<std::string, GLint> uniforms;
};

#endif