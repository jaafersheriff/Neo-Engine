// Texture class
// Contains the OpenGL portion of textures
#pragma once
#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

class Texture {
   public:
      GLuint textureId = 0;
      void init(int, int, char *);
};

#endif