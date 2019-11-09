#pragma once

#include "Texture.hpp"

namespace neo {

    class Texture2D : public Texture {
    public:
        Texture2D(TextureFormat format, glm::uvec2 size, const std::vector<uint8_t>& data = {}) :
            Texture(format, size) 
        {
            upload(data.size() ? data.data() : nullptr);
        }
        
        Texture2D(TextureFormat format, glm::uvec2 size, const uint8_t* data = nullptr) :
            Texture(format, size)
        {
            upload(data);
        }

        virtual void bind() const override {
            CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, mTextureID));
        }

        virtual void upload(const uint8_t* data = nullptr) override {
            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, data));

            /* Set filtering mode for magnification and minimification */
            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFormat.filter));
            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFormat.filter));

            /* Set wrap mode */
            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mFormat.mode));
            CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mFormat.mode));

            /* Unbind */
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

            /* Error check */
            assert(glGetError() == GL_NO_ERROR);
        }

    protected:
        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
        }

    };
}