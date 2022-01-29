#pragma once

#include "Texture.hpp"

namespace neo {

    class Texture1D : public Texture {
    public:
        Texture1D(TextureFormat format, glm::uvec2 size, const void* data) :
            Texture(format, glm::uvec2(size.x, 1)) 
        {
            _applyFormat();
            resize(size);
            _upload(data);
        }

    protected:
        virtual void _bind() const override {
            CHECK_GL(glBindTexture(GL_TEXTURE_1D, mTextureID));
        }

        virtual void _applyFormat() override {
            bind();
            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, mFormat.mFilter));
            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, mFormat.mFilter));

            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, mFormat.mMode));
        }

        virtual void _upload(const void* data) override {
            MICROPROFILE_SCOPEI("Texture1D", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("Texture1D::upload", MP_AUTO);

            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            CHECK_GL(glTexImage1D(GL_TEXTURE_1D, 0, mFormat.mSizedFormat, mWidth, 0, mFormat.mBaseFormat, mFormat.mType, data));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage1D(GL_TEXTURE_1D, 0, mFormat.mSizedFormat, mWidth, 0, mFormat.mBaseFormat, mFormat.mType, 0));
        }

    };
}