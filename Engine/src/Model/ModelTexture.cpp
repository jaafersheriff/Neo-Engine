#include "ModelTexture.hpp"

#define GLEW_STATIC
#include "GL/glew.h"
#include "Util/GLHelper.hpp"

namespace neo {

    void Texture::upload(uint8_t *data, unsigned int mode) {
        /* Generate texture buffer object */
        CHECK_GL(glGenTextures(1, &textureId));

        /* Bind new texture buffer object to active texture */
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, textureId));

        /* Load texture data to GPU */
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));

        /* Generate image pyramid */
        CHECK_GL(glGenerateMipmap(GL_TEXTURE_2D));

        /* Set filtering mode for magnification and minimification */
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));

        /* Set wrap mode */
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLenum) mode));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLenum) mode));
            
        /* LOD */
        CHECK_GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f));

        /* Unbind */
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

        /* Error check */
        assert(glGetError() == GL_NO_ERROR);
    }
}