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
        GLenum type = GL_UNSIGNED_BYTE;
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
            void update(const glm::uvec2 size, const void* data);
            void update(const glm::uvec3 size, const void* data);

            void bind() const;

        protected:
            virtual void _applyFormat() = 0;
            virtual void _resize() = 0;
            virtual void _bind() const = 0;
            virtual void _upload(const void* data) = 0;
    };

}