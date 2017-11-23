/* Abstract parent Shader class */
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
        /* Empty constructor
         * Only used to set GLSL shader names */
        Shader(std::string v = "", std::string f = "") : vShaderName(v), fShaderName(f) { }

        GLuint pid = 0;
        GLint vShaderId;
        GLint fShaderId;

        /* Call parent Shader::init()
         * Add uniforms and attributes to GLSL shaders */
        virtual bool init();

        /* Utility functions */
        void bind();
        void unbind();
        void addAttribute(const std::string &);
        void addUniform(const std::string &);
        GLint getAttribute(const std::string &);
        GLint getUniform(const std::string &);
        void cleanUp();

        /* Load functions */
        void loadBool(const int, const bool) const;
        void loadInt(const int, const int) const;
        void loadFloat(const int, const float) const;
        void loadVec2(const int, const glm::vec2) const;
        void loadVec3(const int, const glm::vec3) const;
        void loadMat4(const int, const glm::mat4*) const;

    protected:
        const std::string vShaderName;
        const std::string fShaderName;
    
    private:
        GLuint createShader(std::string, GLenum);
        std::map<std::string, GLint> attributes;
        std::map<std::string, GLint> uniforms;
};

#endif
