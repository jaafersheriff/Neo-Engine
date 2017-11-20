/* Texture class
 * Contains all texture information */
#pragma once
#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

class Texture {
    public:
        /* Struct describing image
         * Includes pointer to raw image data */
        struct TextureData {
            int width;
            int height;
            int components;
            unsigned char *data;
        };
        
        /* GL texture ID */
        GLuint textureId = 0;

        /* Copy TextureData to GPU */
        void init(TextureData);
};

#endif