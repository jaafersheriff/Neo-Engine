#include "CubeTexture.hpp"

#define GLEW_STATIC
#include "GL/glew.h"

#include "Util/GLHelper.hpp"

namespace neo {

    void CubeTexture::upload(uint8_t *data[6], unsigned int mode) {
        CHECK_GL(glGenTextures(1, &textureId));

        CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, textureId));

        for(int i = 0; i < 6; i++) {
            if (data[i]) {
                CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data[i]));
            }
        }

        CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, (GLenum) mode));
        CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, (GLenum) mode));
        CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, (GLenum) mode));

        CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

        /* Error check */
        assert(glGetError() == GL_NO_ERROR);
    }

}