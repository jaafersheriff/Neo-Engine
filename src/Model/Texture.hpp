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
        enum WRAP_MODE {
            REPEAT,         /* GL_REPEAT - default */
            MIRRORED,       /* GL_MIRRORED_REPEAT  */
            CLAMP,          /* GL_CLAMP_TO_EDGE    */
            CLAMP_TO_BORDER /* GL_CLAMP_TO_BORDER  */
        };

        /* GL texture ID */
        GLuint textureId = 0;

        int width;
        int height;
        int components;

        /* Copy TextureData to GPU */
        void init(const uint8_t *, WRAP_MODE);
};

#endif
