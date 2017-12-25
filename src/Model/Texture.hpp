/* Texture class
 * Contains GL texture ID */
#pragma once
#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <cstdint>

class Texture {
    public:
        /* GL texture ID */
        GLuint textureId = 0;

        int width;
        int height;
        int components;

        /* Copy TextureData to GPU */
        void init(const uint8_t *);
};

#endif
