// Texture class
// Contains the OpenGL portion of textures
#pragma once
#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

class Texture {
    public:
        struct TextureData {
            int width;
            int height;
            int components;
            unsigned char *data;
        };
        
        GLuint textureId = 0;
        void init(TextureData);
};

#endif