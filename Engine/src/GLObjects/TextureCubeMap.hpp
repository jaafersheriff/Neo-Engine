#pragma once

#include "Texture.hpp"

#include "Util/Util.hpp"

namespace neo {

    class TextureCubeMap : public Texture {
    public:

        TextureCubeMap(TextureFormat format, glm::uvec2 size, uint8_t** data = nullptr) :
            Texture(GL_TEXTURE_CUBE_MAP, format, size) {

            upload(data);
        }
        

        virtual void upload(const uint8_t* data = nullptr) override {
        }


        void upload(uint8_t** data) {
            bind();

            // F, B, U, D, R, L
            for (int i = 0; i < 6; i++) {
                if (data && data[i]) {
                    CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, data[i]));
                }
            }

            CHECK_GL(glActiveTexture(GL_TEXTURE0));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
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