#pragma once

#include "Texture.hpp"

namespace neo {

    class Texture1D : public Texture {
    public:
        Texture1D(TextureFormat format, glm::uvec2 size, const std::vector<uint8_t>& data = {}) :
            Texture(format, glm::uvec2(size.x, 1)) 
        {
            bind();
            _applyFormat();
            upload(data.size() ? data.data() : nullptr);
        }

        Texture1D(TextureFormat format, glm::uvec2 size, const std::vector<uint8_t>& data = {}) :
            Texture(format, glm::uvec2(size.x, 1)) 
        {
            bind();
            _applyFormat();
            upload(data.size() ? data.data() : nullptr);
        }

        
        Texture1D(TextureFormat format, glm::uvec2 size, const uint8_t* data = nullptr) :
            Texture(format, glm::uvec2(size.x, 1))
        {
            bind();
            _applyFormat();
            upload(data);
        }

        virtual void _bind() const override {
            CHECK_GL(glBindTexture(GL_TEXTURE_1D, mTextureID));
        }

        virtual void _applyFormat() override {
            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, mFormat.filter));
            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, mFormat.filter));

            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, mFormat.mode));
        }

        virtual void upload(const uint8_t* data = nullptr) override {
            MICROPROFILE_SCOPEI("Texture1D", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("Texture1D::upload", MP_AUTO);

            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            CHECK_GL(glTexImage1D(GL_TEXTURE_1D, 0, mFormat.inputFormat, mWidth, 0, mFormat.format, GL_UNSIGNED_BYTE, data));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

        virtual void upload(const float* data = nullptr) override {
            MICROPROFILE_SCOPEI("Texture1D", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("Texture1D::upload", MP_AUTO);

            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            CHECK_GL(glTexImage1D(GL_TEXTURE_1D, 0, mFormat.inputFormat, mWidth, 0, mFormat.format, GL_FLOAT, data));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

    protected:
        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage1D(GL_TEXTURE_1D, 0, mFormat.inputFormat, mWidth, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
        }

    };
}