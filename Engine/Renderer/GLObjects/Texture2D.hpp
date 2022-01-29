#pragma once

#include "Texture.hpp"

#include "Util/Util.hpp"

namespace neo {

    class Texture2D : public Texture {
    public:
        Texture2D(TextureFormat format, glm::uvec2 size, const void* data) :
            Texture(format, size)
        {
            _applyFormat();
            resize(size);
            _upload(data);
        }

    protected:
        virtual void _bind() const override {
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, mTextureID));
        }

        virtual void _applyFormat() override {
            bind();
            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFormat.mFilter));
            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFormat.mFilter));

            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mFormat.mMode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mFormat.mMode));
        }

        virtual void _upload(const void* data) override {
            MICROPROFILE_SCOPEI("Texture2D", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("Texture2D::upload", MP_AUTO);

            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.mSizedFormat, mWidth, mHeight, 0, mFormat.mBaseFormat, mFormat.mType, data));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.mSizedFormat, mWidth, mHeight, 0, mFormat.mBaseFormat, mFormat.mType, 0));
        }

    };
}