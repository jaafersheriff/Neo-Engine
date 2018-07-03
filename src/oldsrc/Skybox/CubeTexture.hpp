#pragma once
#ifndef _CUBE_TEXTURE_HPP_
#define _CUBE_TEXTURE_HPP_

#include "Model/Texture.hpp"

class CubeTexture : public Texture {
    public:
        void init(uint8_t *data[6]);
};

#endif
