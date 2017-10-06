#include "Texture.hpp"

void Texture::init(int sizeX, int sizeY, char *data) {
   // Set active texture unit 0
   glActiveTexture(GL_TEXTURE0);

   // Generate texture buffer object
   glGenTextures(1, &textureId);

   // Bind new texture buffer object to active texture
   glBindTexture(GL_TEXTURE_2D, textureId);

   // Load texture data to cpu
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sizeX, sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte *) data);

   // Generate image pyramid
   glGenerateMipmap(GL_TEXTURE_2D);

   // Set texture wrap modes for (s, t) coordinates
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
   // Set filtering mode for magnification and minimification
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

   // Set level of detail bias
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f);

   // Unbind
   glBindTexture(GL_TEXTURE_2D, 0);
}