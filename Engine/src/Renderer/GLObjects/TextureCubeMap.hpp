#pragma once

#include "Texture.hpp"

#include "Util/Util.hpp"

#include <array>

namespace neo {

    class TextureCubeMap : public Texture {
    public:

        TextureCubeMap(TextureFormat format, const std::vector<glm::uvec2>& sizes, void** data) :
            Texture(format, glm::uvec2(1)) {

            std::copy_n(sizes.begin(), 6, mSizes.begin());
            bind();
            _applyFormat();
            _upload(sizes, data);
        }

    protected:
        virtual void _upload(const void* data) override {
            NEO_ASSERT(false, "Can't use standard upload for a CubeMap");
        }

        virtual void _bind() const override {
            CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID));
        }

        virtual void _applyFormat() override {
            CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mFormat.filter));
            CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mFormat.filter));

            CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mFormat.mode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mFormat.mode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mFormat.mode));
        }

        void _upload(const std::vector<glm::uvec2>& sizes, void** data) {
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

        virtual void _resize() override {
            bind();
            for (int i = 0; i < 6; i++) {
                CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
                mSizes[i].x = mWidth;
                mSizes[i].y = mHeight;
            }
        }

    private:
        std::array<glm::uvec2, 6> mSizes;

    };
}