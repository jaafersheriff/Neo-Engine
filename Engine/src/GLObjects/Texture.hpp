#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

#include "GLObjects/GLHelper.hpp"

namespace neo {

    struct TextureFormat {
        GLint inputFormat = GL_RGBA;
        GLenum format = GL_RGBA;
        GLint filter = GL_NEAREST;
        GLenum mode = GL_REPEAT;
    };

    class Texture {

        public:

            GLuint mTextureID = 0;
            int mWidth, mHeight, mComponents;
            TextureFormat mFormat;

            /* Upload to GPU */
            virtual void upload(bool = false, uint8_t ** = nullptr) = 0;

            void destroy() {
                CHECK_GL(glDeleteTextures(1, &mTextureID));
            }

            void generateMipMaps() {
                CHECK_GL(glGenerateTextureMipmap(mTextureID));
            }

            virtual void bind() const = 0;
            virtual void resize(const glm::uvec2) = 0;

    };

    class Texture2D : public Texture {

        public:
            void bind() const {
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, mTextureID));
            }

            void resize(const glm::uvec2 size) {
                this->mWidth = size.x;
                this->mHeight = size.y;
                bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
            }

            void upload(bool shouldUpload, uint8_t **data) {
                /* Generate texture buffer object */
                CHECK_GL(glGenTextures(1, &mTextureID));

                /* Bind new texture buffer object to active texture */
                bind();

                /* Load texture data to GPU */
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, shouldUpload ? *data : nullptr));

                /* Set filtering mode for magnification and minimification */
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFormat.filter));
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFormat.filter));

                /* Set wrap mode */
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mFormat.mode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mFormat.mode));

                /* Unbind */
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

                /* Error check */
                assert(glGetError() == GL_NO_ERROR);
            }
    };

    class TextureCubeMap : public Texture {

        public:
            void bind() const {
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));
                CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID));
            }
            
            void upload(bool shouldUpload, uint8_t **data) {
                CHECK_GL(glGenTextures(1, &mTextureID));
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));

                // F, B, U, D, R, L
                if (shouldUpload) {
                    for (int i = 0; i < 6; i++) {
                        if (data[i]) {
                            CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, data[i]));
                        }
                    }
                }

                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mFormat.filter));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mFormat.filter));

                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mFormat.mode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mFormat.mode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mFormat.mode));

                CHECK_GL(glActiveTexture(GL_TEXTURE0));

                /* Error check */
                assert(glGetError() == GL_NO_ERROR);
            }

            void resize(const glm::uvec2 size) {
                // TODO
            }
    };



}