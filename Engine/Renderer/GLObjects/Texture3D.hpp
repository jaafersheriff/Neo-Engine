#pragma once

#include "Texture.hpp"

namespace neo {

    class Texture3D : public Texture {
    public:
        Texture3D(TextureFormat format, glm::uvec3 size, const void* data) :
            Texture(format, size) 
        {
            bind();
            _applyFormat();
            _upload(data);
        }
        
    protected:
        virtual void _bind() const override {
            CHECK_GL(glBindTexture(GL_TEXTURE_3D, mTextureID));
        }

        virtual void _applyFormat() override {
            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, mFormat.mFilter));
            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mFormat.mFilter));

            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, mFormat.mMode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, mFormat.mMode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, mFormat.mMode));
        }

        virtual void _upload(const void* data) override {
            MICROPROFILE_SCOPEI("Texture3D", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("Texture3D::upload", MP_AUTO);

            bind();

            CHECK_GL(glTexImage3D(GL_TEXTURE_3D, 0, mFormat.mInternalFormat, mWidth, mHeight, mDepth, 0, mFormat.mBaseFormat, mFormat.mType, data));

            CHECK_GL(glBindTexture(GL_TEXTURE_3D, 0));
        }

        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage3D(GL_TEXTURE_3D, 0, mFormat.mInternalFormat, mWidth, mHeight, mDepth, 0, mFormat.mBaseFormat, mFormat.mType, 0));
        }

    };
}