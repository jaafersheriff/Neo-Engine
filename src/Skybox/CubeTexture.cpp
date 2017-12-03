#include "CubeTexture.hpp"

void CubeTexture::init(const TextureData td[6]) {

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    for(int i = 0; i < 6; i++) {
        if (td[i].data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, td[i].width, td[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, td[i].data);
        }
    }

    // TODO : update wrap clamp
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
