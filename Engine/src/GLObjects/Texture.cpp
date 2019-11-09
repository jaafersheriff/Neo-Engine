#include "Texture.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    Texture::Texture(TextureFormat format, glm::uvec2 size) :
        Texture(format, glm::uvec3(size.x, size.y, 0)) 
    {}

    Texture::Texture(TextureFormat format, glm::uvec3 size) :
        mFormat(format),
        mWidth(size.x),
        mHeight(size.y),
        mDepth(size.z) {

        CHECK_GL(glGenTextures(1, &mTextureID));
    }

    void Texture::resize(const glm::uvec2 size) {
        resize(glm::uvec3(size, 0));
    }

    void Texture::resize(const glm::uvec3 size) {
        mWidth = size.x;
        mHeight = size.y;
        mDepth = size.z;

        _resize();
    }

    void Texture::destroy() {
        CHECK_GL(glDeleteTextures(1, &mTextureID));
    }

    void Texture::generateMipMaps() {
        CHECK_GL(glGenerateTextureMipmap(mTextureID));
    }
}