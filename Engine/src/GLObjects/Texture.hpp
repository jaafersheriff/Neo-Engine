#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

#include "GLObjects/GLHelper.hpp"

namespace neo {

    class Texture {

        public:
            GLuint mTextureID = 0;
            int mWidth, mHeight, mComponents;
            GLint mInputFormat = GL_RGBA;
            GLenum mFormat = GL_RGBA;
            GLint mFilter = GL_NEAREST;
            GLenum mMode = GL_REPEAT;

            void upload(GLint inFormat, GLenum format, GLint filter, GLenum mode, bool shouldUpload = false, uint8_t **data = 0) {
                mInputFormat = inFormat;
                mFormat = format;
                mFilter = filter;
                mMode = mode;
                _upload(shouldUpload, data);
            }

            void destroy() {
                CHECK_GL(glDeleteTextures(1, &mTextureID));
            }

            void generateMipMaps() {
                CHECK_GL(glGenerateTextureMipmap(mTextureID));
            }

            virtual void bind() const = 0;
            virtual void resize(const glm::uvec2) = 0;

        private:
            /* Upload to GPU */
            virtual void _upload(bool, uint8_t **) = 0;

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
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mInputFormat, mWidth, mHeight, 0, mFormat, GL_UNSIGNED_BYTE, 0));
            }

        private:
            void _upload(bool shouldUpload, uint8_t **data) {
                /* Generate texture buffer object */
                CHECK_GL(glGenTextures(1, &mTextureID));

                /* Bind new texture buffer object to active texture */
                bind();

                /* Load texture data to GPU */
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mInputFormat, mWidth, mHeight, 0, mFormat, GL_UNSIGNED_BYTE, shouldUpload ? *data : nullptr));

                /* Set filtering mode for magnification and minimification */
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter));
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter));

                /* Set wrap mode */
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mMode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mMode));

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
            
            void _upload(bool shouldUpload, uint8_t **data) {
                CHECK_GL(glGenTextures(1, &mTextureID));
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));

                // F, B, U, D, R, L
                if (shouldUpload) {
                    for (int i = 0; i < 6; i++) {
                        if (data[i]) {
                            CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mInputFormat, mWidth, mHeight, 0, mFormat, GL_UNSIGNED_BYTE, data[i]));
                        }
                    }
                }

                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mFilter));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mFilter));

                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mMode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mMode));
                CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mMode));

                CHECK_GL(glActiveTexture(GL_TEXTURE0));

                /* Error check */
                assert(glGetError() == GL_NO_ERROR);
            }

            void resize(const glm::uvec2 size) {
                // TODO
            }
    };



}