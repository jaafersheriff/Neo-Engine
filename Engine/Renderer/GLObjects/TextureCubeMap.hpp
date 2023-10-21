#pragma once

#include "Texture.hpp"

#include <array>

namespace neo {

    class TextureCubeMap : public Texture {
    public:

        TextureCubeMap(TextureFormat format, const std::vector<glm::uvec2>& sizes, void** data) :
            Texture(format, glm::uvec2(1)) {

            std::copy_n(sizes.begin(), 6, mSizes.begin());
            bind();
            _applyFormat();
            _upload(data);
        }

    protected:
        virtual void _upload(const void* data) override {
            NEO_UNUSED(data);
            NEO_ASSERT(false, "Can't use standard upload for a CubeMap");
        }

        virtual void _bind() const override {
            glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);
        }

        virtual void _applyFormat() override {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mFormat.mFilter);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mFormat.mFilter);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mFormat.mMode);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mFormat.mMode);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mFormat.mMode);
        }

        void _upload(void** data) {
            TRACY_GPU();

            NEO_ASSERT(data, "Trying to upload a CubeMap with invalid data");
            bind();

            // F, B, U, D, R, L
            for (int i = 0; i < 6; i++) {
                NEO_ASSERT(data[i], "Trying to upload a CubeMap with invalid data");
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.mInternalFormat, mSizes[i].x, mSizes[i].y, 0, mFormat.mBaseFormat, GL_UNSIGNED_BYTE, data[i]);
            }

            glActiveTexture(GL_TEXTURE0);

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

        virtual void _resize() override {
            bind();
            for (int i = 0; i < 6; i++) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat.mInternalFormat, mWidth, mHeight, 0, mFormat.mBaseFormat, GL_UNSIGNED_BYTE, 0);
                mSizes[i].x = mWidth;
                mSizes[i].y = mHeight;
            }
        }

    private:
        std::array<glm::uvec2, 6> mSizes;

    };
}