
#if 0
#pragma once

#include "Texture.hpp"

#include "Util/Profiler.hpp"
#include "Util/Util.hpp"
#include <tracy/TracyOpenGL.hpp>

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
            glBindTexture(GL_TEXTURE_2D, mTextureID);
        }

        virtual void _applyFormat() override {
            bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFormat.mFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFormat.mFilter);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mFormat.mMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mFormat.mMode);
        }

        virtual void _upload(const void* data) override {
            TRACY_GPU();

            /* Bind texture buffer object to active texture */
            bind();

            /* Load texture data to GPU */
            glTexImage2D(GL_TEXTURE_2D, 0, mFormat.mInternalFormat, mWidth, mHeight, 0, mFormat.mBaseFormat, mFormat.mType, data);

            /* Error check */
            NEO_ASSERT(glGetError() == GL_NO_ERROR, "GLError when uploading Texture");
        }

        virtual void _resize() override {
            bind();
            glTexImage2D(GL_TEXTURE_2D, 0, mFormat.mInternalFormat, mWidth, mHeight, 0, mFormat.mBaseFormat, mFormat.mType, 0);
        }

    };
}
#endif
