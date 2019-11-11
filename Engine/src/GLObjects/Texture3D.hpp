#pragma once

#include "Texture.hpp"

namespace neo {

    class Texture3D : public Texture {
    public:
        Texture3D(TextureFormat format, glm::uvec3 size, const std::vector<uint8_t>& data = {}) :
            Texture(GL_TEXTURE_3D, format, size) 
        {
            upload(data.size() ? data.data() : nullptr);
        }
        
        Texture3D(TextureFormat format, glm::uvec3 size, const uint8_t* data = nullptr) :
            Texture(GL_TEXTURE_3D, format, size)
        {
            upload(data);
        }

        virtual void upload(const uint8_t* data = nullptr) override {
            bind();

            CHECK_GL(glTexImage3D(GL_TEXTURE_3D, 1, mFormat.inputFormat, mWidth, mHeight, mDepth, 0, mFormat.format, GL_UNSIGNED_BYTE, data));

            CHECK_GL(glBindTexture(GL_TEXTURE_3D, 0));
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

    protected:
        virtual void _resize() override {
            bind();
            CHECK_GL(glTexImage3D(GL_TEXTURE_3D, 0, mFormat.inputFormat, mWidth, mHeight, mDepth, 0, mFormat.format, GL_UNSIGNED_BYTE, 0));
        }


    };
}