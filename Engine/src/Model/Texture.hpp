#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

#include "Util/GLHelper.hpp"

namespace neo {

    class Texture {

        public:
            virtual ~Texture() {
                CHECK_GL(glDeleteTextures(1, &textureId));
            }

            void generateMipMaps() {
                CHECK_GL(glGenerateTextureMipmap(textureId));
            }

            GLuint textureId = 0;
            int width, height, components;

            /* Upload to GPU */
            void upload(GLint, GLenum, GLint, GLenum, uint8_t * = nullptr);
            void uploadCubeMap(uint8_t **);
            // TODO - virtual void upload(GLint, GLenum, GLint, GLenum, uint8_t ** = 0) = 0;

    };

}