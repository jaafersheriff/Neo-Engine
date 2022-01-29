#pragma once

#include <GL/glew.h>
#include "glm/glm.hpp"

#include <unordered_map>
#include <map>
#include <string>
#include <iostream>

namespace neo {

    class Texture;

    enum class ShaderStage {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        TESSELLATION_CONTROL,
        TESSELLATION_EVAL,
        COMPUTE
    };

    struct ShaderSource {
        GLint id = 0;
        std::string file = "";
        const char* source = nullptr;
        std::string processedSource;
    };

    class Shader {

        public:
            Shader(const std::string &name);
            // Base vertex/fragment
            Shader(const std::string &name, const std::string& vertexFile, const std::string& fragmentFile);
            Shader(const std::string &name, const char* vertexSource, const char* fragmentSource);
            Shader(Shader&& rhs) = default;
            virtual ~Shader() = default;

            virtual void render() {}
            virtual void imguiEditor() {}
            bool mActive = true;

            /* Names */
            const std::string mName = 0;

            /* Utility functions */
            void init();
            void bind();
            void unbind();
            void reload();
            void cleanUp();

            /* Parent load functions */
            void loadUniform(const std::string &, const bool) const;          // bool
            void loadUniform(const std::string &, const int) const;           // int
            void loadUniform(const std::string &, const double) const;        // double
            void loadUniform(const std::string &, const float) const;         // float
            void loadUniform(const std::string &, const glm::vec2&) const;   // vec2
            void loadUniform(const std::string &, const glm::ivec2&) const;  // ivec2
            void loadUniform(const std::string &, const glm::vec3&) const;   // vec3
            void loadUniform(const std::string &, const glm::vec4&) const;   // vec4
            void loadUniform(const std::string &, const glm::mat3&) const;   // mat3
            void loadUniform(const std::string &, const glm::mat4&) const;   // mat4
            void loadTexture(const std::string &, const Texture &) const;     // texture

            /* Get shader location */
            GLint getAttribute(const std::string &) const;
            GLint getUniform(const std::string &) const;

        protected: 
            void _attachStage(ShaderStage type, const std::string& file);
            void _attachStage(ShaderStage type, const char* source);

        private:
            /* GLSL shader attributes */
            GLuint mPID = 0;

            std::unordered_map<ShaderStage, ShaderSource> mStages;

            std::map<std::string, GLint> mAttributes;
            std::map<std::string, GLint> mUniforms;

            std::string _getFullPath(const std::string&);

            GLint _getGLShaderStage(ShaderStage);

            GLuint _compileShader(GLenum, const char *);
            std::string _processShader(const char *);
            void _findAttributesAndUniforms(const char *);
            void _addAttribute(const std::string &);
            void _addUniform(const std::string &);
    };
}