#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>

namespace neo {

    class Texture {

        public:
            virtual ~Texture() {
                // TODO - delete textureId
            }

            GLuint textureId = 0;

            int width, height, components;

            /* Upload to GPU */
            void upload(GLint, GLenum, GLint, GLenum, uint8_t * = nullptr);
            void uploadCubeMap(uint8_t **);
    };


}