#include "Texture.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLObjects/GLHelper.hpp"

#include "Util/Util.hpp"

namespace neo {

    Texture::Texture(GLuint textureType, TextureFormat format, glm::uvec2 size) :
        Texture(textureType, format, glm::uvec3(size.x, size.y, 0)) 
    {}

    Texture::Texture(GLuint textureType, TextureFormat format, glm::uvec3 size) :
        mTextureType(textureType),
        mFormat(format),
        mWidth(size.x),
        mHeight(size.y),
        mDepth(size.z) {

        CHECK_GL(glGenTextures(1, &mTextureID));
        bind();

        applyFormat();
    }

    void Texture::bind() const {
        CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));
        CHECK_GL(glBindTexture(mTextureType, mTextureID));
    }

    void Texture::applyFormat() {
        CHECK_GL(glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, mFormat.filter));
        CHECK_GL(glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, mFormat.filter));

        CHECK_GL(glTexParameteri(mTextureType, GL_TEXTURE_WRAP_S, mFormat.mode));
        if (mTextureType != GL_TEXTURE_1D) {
            CHECK_GL(glTexParameteri(mTextureType, GL_TEXTURE_WRAP_T, mFormat.mode));

            if (mTextureType != GL_TEXTURE_2D) {
                CHECK_GL(glTexParameteri(mTextureType, GL_TEXTURE_WRAP_R, mFormat.mode));
            }
	    }
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

    void Texture::update(const glm::uvec2 size, const uint8_t* data) {
        update(glm::uvec3(size, 0), data);
    }

    void Texture::update(const glm::uvec3 size, const uint8_t* data) {
        NEO_ASSERT(data != nullptr, "Attempting to update a texture with nothing");
        resize(size);
        upload(data);
    }

    void Texture::update(const glm::uvec2 size, const float* data) {
        update(glm::uvec3(size, 0), data);
    }

    void Texture::update(const glm::uvec3 size, const float* data) {
        NEO_ASSERT(data != nullptr, "Attempting to update a texture with nothing");
        resize(size);
        upload(data);
    }

    void Texture::destroy() {
        CHECK_GL(glDeleteTextures(1, &mTextureID));
    }

    void Texture::generateMipMaps() {
        CHECK_GL(glGenerateTextureMipmap(mTextureID));
    }
}
