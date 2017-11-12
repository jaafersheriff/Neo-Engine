#include "CubeTexture.hpp"

void CubeTexture::init(Loader &loader, const std::string texName1, const std::string texName2, const std::string texName3, const std::string texName4, const std::string texName5, const std::string texName6) {
   Texture::TextureData td[6];
   td[0] = loader.getTextureData(texName1);
   td[1] = loader.getTextureData(texName2);
   td[2] = loader.getTextureData(texName3);
   td[3] = loader.getTextureData(texName4);
   td[4] = loader.getTextureData(texName5);
   td[5] = loader.getTextureData(texName6);

   glActiveTexture(GL_TEXTURE0);

   glGenTextures(1, &textureId);

   glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

   for(int i = 0; i < 6; i++) {
      if (td[i].data) {
         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, td[i].width, td[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, td[i].data);
      }
   }

   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}