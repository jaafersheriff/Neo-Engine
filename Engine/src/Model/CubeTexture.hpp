#pragma once

#include "Texture.hpp"

namespace neo {

    class CubeTexture : public Texture {

        public:

            virtual void upload(uint8_t **, unsigned int);

    };
}