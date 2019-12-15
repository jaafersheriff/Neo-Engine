#pragma once

#include "Texture.hpp"

#include "Util/Util.hpp"

#include <array>

namespace neo {

    class TextureCubeMap : public Texture {
    public:

        TextureCubeMap(TextureFormat format, const std::vector<glm::uvec2>& sizes, uint8_t** data = nullptr) :
            Texture(GL_TEXTURE_CUBE_MAP, format, glm::uvec2(1)) {

            std::copy_n(sizes.begin(), 6, mSizes.begin());
            upload(sizes, data);
        }
        

        virtual void upload(const uint8_t* data = nullptr) override {
        }

        virtual void upload(const float* data = nullptr) override {
        }

        void upload(const std::vector<glm::uvec2>& sizes, uint8_t** data) {
            MICROPROFILE_SCOPEI("TextureCube", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("TextureCube::upload", MP_AUTO);

            NEO_ASSERT(data, "Trying to upload a CubeMap with invalid data");
            bind();

            // F, B, U, D, R, L
            for (int i = 0; i < 6; i++) {
                NEO_ASSERT(data[i], "Trying to upload a CubeMap with invalid data");
                CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.inputFormat, mSizes[i].x, mSizes[i].y, 0, mFormat.format, GL_UNSIGNED_BYTE, data[i]));
            }

            CHECK_GL(glActiveTexture(GL_TEXTURE0));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

    protected:
        std::array<glm::uvec2, 6> mSizes;

        virtual void _resize() override {
            bind();
            for (int i = 0; i < 6; i++) {
                CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
                mSizes[i].x = mWidth;
                mSizes[i].y = mHeight;
            }
        }

    };
}