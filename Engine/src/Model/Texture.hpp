#pragma once

#include <glm/glm.hpp>

namespace neo {

    class Texture {

        public:

            unsigned int textureId;

            int width, height, components;

            /* Upload to GPU */
            void upload(uint8_t **, unsigned int);
            void uploadCubeMap(uint8_t **, unsigned int);
    };


}