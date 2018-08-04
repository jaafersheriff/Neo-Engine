#pragma once

#include <glm/glm.hpp>

namespace neo {

    struct Material {
        Material() :
            ambient(1.f),
            diffuse(glm::vec3(0.75f)),
            specular(glm::vec3(0.75f)),
            shine(16.f)
        {}

        Material(const float amb, const glm::vec3 & dif, const glm::vec3 & spec, const float shine) :
            ambient(amb),
            diffuse(dif),
            specular(spec),
            shine(shine)
        {}

        float ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float shine;
    };

    struct Texture {
        unsigned int textureId;

        int width, height, components;

        /* Upload to GPU */
        void upload(uint8_t *, unsigned int);
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

            const Texture * getTexture() const { return texture; }
            Material * getMaterial() { return &material; }

        private:
            const Texture * texture;
            Material material;
    };
}