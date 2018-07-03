/* Model Texture class
 * Contains reference to an optionally loaded texture, texture properties, and material properties
 * Rendering will based on material properties if loaded texture doesn't exist */
#pragma once
#ifndef _MODEL_TEXTURE_HPP_
#define _MODEL_TEXTURE_HPP_

#include "Texture.hpp"

#include "glm/glm.hpp"

class ModelTexture {
    public:
        /* Constructors */
        ModelTexture() 
        { }
    
        ModelTexture(const float ambient, const glm::vec3 diffuse, const glm::vec3 specular) :
            ambientColor(ambient),
            diffuseColor(diffuse),
            specularColor(specular)
        {}


        ModelTexture(Texture *texture, const float ambient, const glm::vec3 diffuse, const glm::vec3 specular) :
            texture(texture), 
            ambientColor(ambient),
            diffuseColor(diffuse),
            specularColor(specular)
        {}

        /* Texture properties */
        // TODO : comments describing what each of these do
        Texture *texture = nullptr;
        float shineDamper = 1.f;
        bool hasTranspency = false;
        bool hasFakeLighting = false;
        int numRows = 1;

        /* Material properties */
        float ambientColor = 0.f;
        glm::vec3 diffuseColor = glm::vec3(0.f);
        glm::vec3 specularColor = glm::vec3(0.f);
};

#endif