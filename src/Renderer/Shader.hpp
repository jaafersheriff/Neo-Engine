/* Abstract parent Shader class 
 * Every feature will have its own derived shader */
#pragma once
#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include "MasterRenderer.hpp"

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

        /* Subshaders need a reference to the render target data structure */
        // TODO : templates 

        /* Call parent Shader::init()
         * Add uniforms and attributes to GLSL shaders */
        virtual bool init();

        /* Type */
        MasterRenderer::ShaderTypes type;

        /* Utility functions */
        void bind();
        void unbind();
        void cleanUp();
        void unloadMesh();
        void unloadTexture(int);

        /* Render functions */
        // TODO : give worlds their own global map 
        virtual void render() = 0;

        /* Parent load functions */
        void loadBool(const int, const bool) const;
        void loadInt(const int, const int) const;
        void loadFloat(const int, const float) const;
        void loadVec2(const int, const glm::vec2) const;
        void loadVec3(const int, const glm::vec3) const;
        void loadMat4(const int, const glm::mat4*) const;

        /* Get shader location */
        GLint getAttribute(const std::string &);
        GLint getUniform(const std::string &);
    protected:
        /* GLSL shader names */
        const std::string vShaderName;
        const std::string fShaderName;

        /* Shared GLSL utility functions */
        void addAttribute(const std::string &);
        void addUniform(const std::string &);

        /* Virtual function for adding all uniforms and attributes */
        virtual void addAllLocations() = 0;
    private:    
        /* GLSL shader attributes */
        GLuint pid = 0;
        GLint vShaderId;
        GLint fShaderId;
        std::map<std::string, GLint> attributes;
        std::map<std::string, GLint> uniforms;

        /* GLSL utility functions */
        GLuint createShader(std::string, GLenum);

};

#endif
