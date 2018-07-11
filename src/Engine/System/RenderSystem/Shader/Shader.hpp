#pragma once

#include "System/RenderSystem/GLHelper.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <map>
#include <string>
#include <iostream>

namespace neo {

    class Shader {

    public:
        Shader(const std::string &, const std::string &, const std::string &, const std::string &);
        Shader(const std::string &, const std::string &, const std::string &);

        /* Utility functions */
        void bind();
        void unbind();
        void cleanUp();
        void addAttribute(const std::string &);
        void addUniform(const std::string &);

        /* Parent load functions */
        void loadBool(const int, const bool) const;
        void loadInt(const int, const int) const;
        void loadFloat(const int, const float) const;
        void loadVector(const int, const glm::vec2 &) const;
        void loadVector(const int, const glm::vec3 &) const;
        void loadVector(const int, const glm::vec4 &) const;
        void loadMatrix(const int, const glm::mat4 &) const;
        void loadMatrix(const int, const glm::mat3 &) const;

        /* Get shader location */
        GLint getAttribute(const std::string &);
        GLint getUniform(const std::string &);

    private:
        /* GLSL shader attributes */
        GLuint pid = 0;
        GLint vShaderId;
        GLint fShaderId;
        GLint gShaderId;
        std::map<std::string, GLint> attributes;
        std::map<std::string, GLint> uniforms;

        GLuint compileShader(GLenum, const std::string &, const std::string &);
        void findAttributesAndUniforms(const std::string &, const std::string &);
    };
}