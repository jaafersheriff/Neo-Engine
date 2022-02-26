#include "Texture.hpp"

#include "GL/glew.h"

#include "Renderer/GLObjects/GLHelper.hpp"

#include "Util/Util.hpp"

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

    void Texture::bind() const {
        CHECK_GL(glActiveTexture(GL_TEXTURE0 + mTextureID));
        _bind();
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

    void Texture::update(const glm::uvec2 size, const void* data) {
        update(glm::uvec3(size, 0), data);
    }

    void Texture::update(const glm::uvec3 size, const void* data) {
        NEO_ASSERT(data != nullptr, "Attempting to update a texture with nothing");
        resize(size);
        _upload(data);
    }

    void Texture::destroy() {
        CHECK_GL(glDeleteTextures(1, &mTextureID));
    }

    void Texture::generateMipMaps() {
        CHECK_GL(glGenerateTextureMipmap(mTextureID));
    }
}
