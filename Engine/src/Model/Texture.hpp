#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

#include "Util/GLHelper.hpp"

namespace neo {

    class Texture {

        public:
            void generateMipMaps() {
                CHECK_GL(glGenerateTextureMipmap(textureId));
            }

            GLuint textureId = 0;
            int width, height, components;

            /* Upload to GPU */
            virtual void upload(GLint, GLenum, GLint, GLenum, uint8_t ** = 0) = 0;

            virtual void bind() = 0;

    };

    class Texture2D : public Texture {

        public:
            void bind() {
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureId));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, textureId));
            }

            void upload(GLint inFormat, GLenum format, GLint filter, GLenum mode, uint8_t **data) {
                /* Generate texture buffer object */
                CHECK_GL(glGenTextures(1, &textureId));

                /* Bind new texture buffer object to active texture */
                bind();

                /* Load texture data to GPU */
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, inFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data[0]));

                /* Set filtering mode for magnification and minimification */
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));

                /* Set wrap mode */
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode));

                /* Unbind */
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

                /* Error check */
                assert(glGetError() == GL_NO_ERROR);
            }
    };

    class TextureCubeMap : public Texture {

        public:
            void bind() {

            }
            
            void upload(GLint inFormat, GLenum format, GLint filter, GLenum mode, uint8_t **data) {
                CHECK_GL(glGenTextures(1, &textureId));
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureId));

                // F, B, U, D, R, L
                for(int i = 0; i < 6; i++) {
                    if (data[i]) {
                        CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inFormat, this->width, this->height, 0, format, GL_UNSIGNED_BYTE, data[i]));
                    }
                }

                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter));

                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mode));

                CHECK_GL(glActiveTexture(GL_TEXTURE0));

                /* Error check */
                assert(glGetError() == GL_NO_ERROR);
            }
    };



}