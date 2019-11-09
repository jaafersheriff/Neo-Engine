#pragma once

#include "Texture.hpp"

namespace neo {

    class Texture3D : public Texture {
    public:
        Texture3D(TextureFormat format, glm::uvec3 size, const std::vector<uint8_t>& data = {}) :
            Texture(format, size) 
        {
            upload(data.size() ? data.data() : nullptr);
        }
        
        Texture3D(TextureFormat format, glm::uvec3 size, const uint8_t* data = nullptr) :
            Texture(format, size)
        {
            upload(data);
        }

        virtual void bind() const override {
            CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));
            CHECK_GL(glBindTexture(GL_TEXTURE_3D, mTextureID));
        }

        virtual void upload(const uint8_t* data = nullptr) override {
            bind();

            CHECK_GL(glTexImage3D(GL_TEXTURE_3D, 1, mFormat.inputFormat, mWidth, mHeight, mDepth, 0, mFormat.format, GL_UNSIGNED_BYTE, data));

            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, mFormat.filter));
            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mFormat.filter));

            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, mFormat.mode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, mFormat.mode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, mFormat.mode));

            CHECK_GL(glBindTexture(GL_TEXTURE_3D, 0));
            assert(glGetError() == GL_NO_ERROR);
        }

    protected:
        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage3D(GL_TEXTURE_3D, 0, mFormat.inputFormat, mWidth, mHeight, mDepth, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
        }


    };
}