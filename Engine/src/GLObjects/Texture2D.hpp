#pragma once

#include "Texture.hpp"

#include "Util/Util.hpp"

namespace neo {

    class Texture2D : public Texture {
    public:
        Texture2D(TextureFormat format, glm::uvec2 size, const std::vector<uint8_t>& data = {}) :
            Texture(GL_TEXTURE_2D, format, size) 
        {
            upload(data.size() ? data.data() : nullptr);
        }
        
        Texture2D(TextureFormat format, glm::uvec2 size, const uint8_t* data = nullptr) :
            Texture(GL_TEXTURE_2D, format, size)
        {
            upload(data);
        }


        virtual void upload(const uint8_t* data = nullptr) override {
            MICROPROFILE_SCOPEI("Texture2D", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("Texture2D::upload", MP_AUTO);

            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, data));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

        virtual void upload(const float* data = nullptr) override {
            MICROPROFILE_SCOPEI("Texture2D", "upload", MP_AUTO);
            MICROPROFILE_SCOPEGPUI("Texture2D::upload", MP_AUTO);

            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_FLOAT, data));

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

    protected:
        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, mFormat.inputFormat, mWidth, mHeight, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
        }

    };
}