#include "Texture.hpp"

void Texture::init(TextureData td) {
    // Set active texture unit 0
    glActiveTexture(GL_TEXTURE0);

    // Generate texture buffer object
    glGenTextures(1, &textureId);

    // Bind new texture buffer object to active texture
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Load texture data to cpu
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, td.width, td.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, td.data);

    // Generate image pyramid
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Set filtering mode for magnification and minimification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f); 

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
}