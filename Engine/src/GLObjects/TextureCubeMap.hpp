#pragma once

#include "Texture.hpp"

namespace neo {

    class TextureCubeMap : public Texture {
    public:

        TextureCubeMap(TextureFormat format, glm::uvec2 size, uint8_t** data = nullptr) :
            Texture(format, size) {

            upload(data);
        }
        

        virtual void bind() const override {
            CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));
            CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID));
        }

        virtual void upload(const uint8_t* data = nullptr) override {
        }


        void upload(uint8_t** data) {
            // F, B, U, D, R, L
            for (int i = 0; i < 6; i++) {
                if (data && data[i]) {
                    CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, data[i]));
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

    protected:
        virtual void _resize() override {
            bind();
            for (int i = 0; i < 6; i++) {
                CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
            }
        }

    };
}