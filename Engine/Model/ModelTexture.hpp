#pragma once

#include <glm/glm.hpp>
#define GLEW_STATIC
#include "GL/glew.h"

namespace neo {

    struct Material {
        Material() :
            diffuse(glm::vec3(0.75f)),
            specular(glm::vec3(0.75f)),
            shine(16.f)
        {}

        Material(const glm::vec3 & dif, const glm::vec3 & spec, const float shine) :
            diffuse(dif),
            specular(spec),
            shine(shine)
        {}

        glm::vec3 diffuse;
        glm::vec3 specular;
        float shine;
    };

    struct Texture {
        GLuint textureId;

        int width, height, components;
    };

    class ModelTexture {
        public:
        
            ModelTexture() :
                texture(nullptr),
                material()
            {}

            ModelTexture(const Texture *tex) :
                texture(tex),
                material()
            {}

            ModelTexture(const Material &mat) :
                texture(nullptr),
                material(mat)
            {}

            ModelTexture(const Texture *tex, const Material &mat) :
                texture(tex),
                material(mat)
            {}

        private:
            const Texture * texture;
            const Material material;
    };
}