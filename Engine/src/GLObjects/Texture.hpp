#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLObjects/GLHelper.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace neo {

    struct TextureFormat {
        GLint inputFormat = GL_RGBA;
        GLenum format = GL_RGBA;
        GLint filter = GL_NEAREST;
        GLenum mode = GL_REPEAT;
    };

    class Texture {

        public:

            /* Constructor */
            Texture(TextureFormat format, glm::uvec2 size);
            Texture(TextureFormat format, glm::uvec3 size);

            /* Delete copy constructors */
            Texture(const Texture &) = delete;
            Texture & operator=(const Texture &) = delete;
            Texture(Texture &&) = default;
            Texture & operator=(Texture &&) = default;

            /* Params */
            GLuint mTextureID = 0;
            unsigned mWidth = 1;
            unsigned mHeight = 1;
            unsigned mDepth = 0;
            TextureFormat mFormat;

            /* Remove */
            void destroy();

            void generateMipMaps();
            void resize(const glm::uvec2 size);
            void resize(const glm::uvec3 size);

            /* Upload to GPU */
            virtual void upload(const uint8_t* data = nullptr) = 0;

            virtual void bind() const = 0;

        protected:
            virtual void _resize() = 0;
    };

}