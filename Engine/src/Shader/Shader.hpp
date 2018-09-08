#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <map>
#include <string>
#include <iostream>

namespace neo {

    class CameraComponent;

    class Shader {

        public:
            Shader(const std::string &);
            Shader(const std::string &, const std::string &, const std::string &);
            Shader(const std::string &, const std::string &, const std::string &, const std::string &);
            Shader(const std::string &, const char *, const char *);
            Shader(const std::string &, const char *, const char *, const char *);
            virtual ~Shader() = default;

            virtual void render(const CameraComponent &) = 0;
            bool active = true;

            /* Names */
            const std::string name = 0;

            /* Utility functions */
            void bind();
            void unbind();
            void cleanUp();
            void addAttribute(const std::string &);
            void addUniform(const std::string &);

            /* Parent load functions */
            void loadUniform(const std::string &, const bool) const;          // bool
            void loadUniform(const std::string &, const int) const;           // int
            void loadUniform(const std::string &, const GLuint) const;        // GLuint
            void loadUniform(const std::string &, const float) const;         // float
            void loadUniform(const std::string &, const glm::vec2 &) const;   // vec2
            void loadUniform(const std::string &, const glm::vec3 &) const;   // vec3
            void loadUniform(const std::string &, const glm::vec4 &) const;   // vec4
            void loadUniform(const std::string &, const glm::mat3 &) const;   // mat3
            void loadUniform(const std::string &, const glm::mat4 &) const;   // mat4

            /* Get shader location */
            GLint getAttribute(const std::string &) const;
            GLint getUniform(const std::string &) const;

        private:
            /* GLSL shader attributes */
            GLuint pid = 0;
            GLint vShaderId = 0;
            GLint fShaderId = 0;
            GLint gShaderId = 0;
            std::map<std::string, GLint> attributes;
            std::map<std::string, GLint> uniforms;

            GLuint compileShader(GLenum, const char *);
            void findAttributesAndUniforms(const char *);
    };
}