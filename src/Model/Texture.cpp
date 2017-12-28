#include "Texture.hpp"

/* Copy TextureData to GPU */
void Texture::init(const uint8_t *data, WRAP_MODE mode) {
    /* Set active texture unit 0 */
    glActiveTexture(GL_TEXTURE0);

    /* Generate texture buffer object */
    glGenTextures(1, &textureId);

    /* Bind new texture buffer object to active texture */
    glBindTexture(GL_TEXTURE_2D, textureId);

    /* Load texture data to GPU */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    /* Generate image pyramid */
    glGenerateMipmap(GL_TEXTURE_2D);
    
    /* Set filtering mode for magnification and minimification */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    /* Set wrap mode */
    switch (mode) {
        case MIRRORED:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            break;
        case CLAMP:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        case CLAMP_TO_BORDER:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            break;
        case REPEAT:
        default:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
    }

    /* LOD */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f); 

    /* Unbind */
    glBindTexture(GL_TEXTURE_2D, 0);
}
